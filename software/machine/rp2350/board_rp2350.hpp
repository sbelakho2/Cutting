#pragma once
// RP2350 Board implementation for the selected Raspberry Pi Pico 2 (SC1631).
// GPIO assignment and electrical polarity are fixed in shared/controls_spec.hpp.
#include "board.hpp"
#include "settings_journal.hpp"
#include <atomic>
#include <cstdint>
#include "hardware/pio.h"

namespace starz::fw::rp2350 {

class BoardRp2350 final : public Board {
public:
    BoardRp2350();

    // Initializes all GPIOs, PIO programs, USB CDC, flash journal and watchdog.
    // It must run before Controller::init().
    void initialize();
    // Drain both encoder PIO FIFOs. Call in every iteration of the foreground
    // loop; at maximum feed speed each encoder produces at most 20,480 edges/s.
    void service();

    std::uint64_t now_us() override;
    Inputs read_inputs() override;
    void write_outputs(const Outputs& outputs) override;

    void set_direction(Axis axis, bool forward) override;
    std::uint32_t step_fifo_level(Axis axis) override;
    void push_steps(Axis axis, const StepRun& run) override;
    bool steps_idle(Axis axis) override;
    void abort_steps(Axis axis) override;

    std::int64_t encoder_count(std::uint8_t side) override;
    bool encoder_healthy(std::uint8_t side) override;
    std::uint16_t boot_faults() override;
    std::uint16_t latched_faults() override;
    bool clear_latched_faults() override;

    bool load_settings(void* dst, std::uint32_t len) override;
    bool store_settings(const void* src, std::uint32_t len) override;
    void watchdog_feed() override;

private:
    struct StepSm {
        PIO pio;
        std::uint8_t sm;
        std::uint8_t pul_pin;
        std::uint8_t dir_pin;
        std::uint16_t program_offset;
        std::uint64_t busy_until_us;
    };

public: // Exposed solely for target compile-time storage-layout checks.
    using FlashRecord = journal::Record;
    static_assert(sizeof(FlashRecord) == 60, "Flash record layout changed; review journal page allocation");

    static constexpr std::uint32_t flash_magic = journal::record_magic;
    static constexpr std::uint32_t flash_commit = journal::committed;
    static constexpr std::uint32_t physical_flash_bytes = 4u * 1024u * 1024u;
    static constexpr std::uint32_t settings_slots = 2;

private:
    static void gpio_irq(uint gpio, std::uint32_t events);
    void emergency_disable(std::uint16_t reason);
    void restart_step_sm(StepSm& sm, bool enable);
    void restart_encoder_sm(std::uint8_t side);
    void service_encoder(std::uint8_t side);
    void latch_fault(std::uint16_t reason);
    bool field_safety_ok() const;

    static const FlashRecord* flash_slot(std::uint32_t slot);
    static const std::uint32_t* flash_commit_slot(std::uint32_t slot);

    StepSm step_[ctrl::axis::count]{};
    std::uint16_t step_offset_[2]{};
    std::uint16_t quadrature_offset_ = 0;
    std::int64_t encoder_count_[2] = {0, 0};
    std::uint8_t encoder_last_[2] = {0, 0};
    bool encoder_ok_[2] = {true, true};
    std::atomic<std::uint16_t> latched_faults_{0};
    std::uint16_t boot_faults_ = 0;
    std::int32_t active_slot_ = -1;
    std::uint32_t generation_ = 0;
    bool initialized_ = false;

    static BoardRp2350* instance_;
};

} // namespace starz::fw::rp2350
