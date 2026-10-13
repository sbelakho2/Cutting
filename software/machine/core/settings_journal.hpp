#pragma once
// Power-loss-safe persistent-settings journal contract shared by the RP2350
// adapter and host regression tests. The target writes one record page and
// only then writes this record's distinct commit page.
#include "crc16.hpp"
#include "types.hpp"

#include <cstddef>
#include <cstdint>

namespace starz::fw::journal {

inline constexpr std::uint32_t record_magic = 0x53545A46u; // "STZF"
inline constexpr std::uint32_t committed = 0x00000000u;

struct Record {
    std::uint32_t magic;
    std::uint32_t generation;
    Settings settings;
    std::uint32_t record_crc;
};

static_assert(sizeof(Record) == 76, "Journal record layout changed; review target flash allocation");

inline std::uint32_t record_crc(const Record& record) noexcept {
    return crc16(reinterpret_cast<const std::uint8_t*>(&record), offsetof(Record, record_crc));
}

inline bool valid(const Record& record, std::uint32_t commit_latch) noexcept {
    const Settings defaults{};
    const std::uint32_t settings_crc = crc16(reinterpret_cast<const std::uint8_t*>(&record.settings), offsetof(Settings, crc));
    return record.magic == record_magic && commit_latch == committed && record.record_crc == record_crc(record) &&
           record.settings.magic == defaults.magic && record.settings.crc == settings_crc &&
           record.settings.counts_per_metre[0] > 0 && record.settings.counts_per_metre[1] > 0;
}

// Unsigned generation numbers compare correctly across a single 32-bit wrap,
// provided no valid records differ by 2^31 or more generations.
inline bool newer(std::uint32_t candidate, std::uint32_t incumbent) noexcept {
    return static_cast<std::int32_t>(candidate - incumbent) > 0;
}

// Returns -1 when no explicitly committed valid record exists.
inline std::int32_t select(const Record* records, const std::uint32_t* commit_latches, std::uint32_t count) noexcept {
    if (records == nullptr || commit_latches == nullptr) return -1;
    std::int32_t selected = -1;
    std::uint32_t selected_generation = 0;
    for (std::uint32_t slot = 0; slot < count; ++slot) {
        const Record& record = records[slot];
        if (!valid(record, commit_latches[slot])) continue;
        if (selected < 0 || newer(record.generation, selected_generation)) {
            selected = static_cast<std::int32_t>(slot);
            selected_generation = record.generation;
        }
    }
    return selected;
}

} // namespace starz::fw::journal