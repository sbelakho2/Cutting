#pragma once
// STARZ machine host link, revision 7 (screenless machine; the host is an
// optional remote panel over USB-CDC and is never required for motion).
// Revision 7 adds the downstream piece-station telemetry, fixture-mode state,
// the waste-chute channel and the three-station calibration payload (M27/M30).
// Frame: A5 5A | msg:u8 | seq:u8 | len:u8 | payload[len] | crc16 (CCITT-FALSE
// over msg..payload, big-endian on the wire). Multi-byte payload fields are
// little-endian. The largest frame is the STATUS payload.
#include <cstddef>
#include <cstdint>
#include "types.hpp"
#include "controller.hpp"

namespace starz::fw::proto {

inline constexpr std::uint8_t sync0 = 0xA5, sync1 = 0x5A;
inline constexpr std::size_t max_payload = 128;
inline constexpr std::size_t max_frame = 5 + max_payload + 2;
inline constexpr std::uint8_t protocol_version = 7;
static_assert(sizeof(ctrl::revision) + 1u <= max_payload); // version byte plus NUL-terminated revision

enum class Msg : std::uint8_t {
    Hello = 0x01, HelloAck = 0x81,
    GetStatus = 0x02, Status = 0x82,
    LoadJob = 0x10, ClearJob = 0x11, Start = 0x12, Stop = 0x13, Reset = 0x14, Home = 0x15,
    Jog = 0x16, SetCalibration = 0x17, Approve = 0x18, CommissionAxis = 0x19, SetPartClear = 0x1A,
    SetFixtureMode = 0x1B,
    Ack = 0x90, Nack = 0x91,
};

// LoadJob payload (24 bytes): length_um i32, strip_lead_um i32, strip_tail_um i32,
// strip_depth_um i32, wire_diameter_um i32, quantity u32.
// Jog payload (10 bytes): axis u8, forward u8, distance_um i32, speed_um_s i32.
// SetCalibration payload (12 bytes): W0 i32, W1 i32, piece-station D i32.
// CommissionAxis payload (2 bytes): axis u8, logical_forward u8.
// SetPartClear payload (1 byte): enable u8 (0/1). Disabling requires fixture mode.
// SetFixtureMode payload (1 byte): enable u8 (0/1). RAM-only service state.
// Ack payload (1 byte): CommandResult of the request. Nack payload (1 byte): CommandResult.
// HelloAck payload: version u8, revision string (NUL-terminated, <= 40).
// Status payload (124 bytes), all little-endian:
//   state u8, phase u8, faults u32, inputs u16 (bit0 start, 1 stop_ok, 2 reset,
//   3 safety_ok, 4 home_front, 5 home_rear, 8..11 alm[0..3], 12 part_clear,
//   13 waste_clear),
//   outputs u8 (bit0 green, 1 red, 2 buzzer, 3 drives_disabled, 4 approved,
//   5 waste_sweep),
//   job_loaded u8, pieces_done u32, pieces_accepted u32, pieces_scrap u32,
//   last_residual_um i32, worst_residual_um i32, blade_um[2] i32,
//   feed_um[2] i64, encoder_counts[2] i64, counts_per_metre[2] i32,
//   uptime_ms u32, job: length_um i32, strip_lead_um i32, strip_tail_um i32,
//   strip_depth_um i32, wire_diameter_um i32, quantity u32,
//   piece_encoder_counts i64, pieces_fixture u32, piece counts_per_metre i32,
//   fixture_mode u8, part_clear_check u8.
inline constexpr std::size_t load_job_len = 24, jog_len = 10, calibration_len = 12,
                             commission_len = 2, set_part_clear_len = 1, set_fixture_len = 1,
                             status_len = 124;
static_assert(status_len <= max_payload);

struct Frame {
    Msg msg{};
    std::uint8_t seq = 0;
    std::uint8_t len = 0;
    std::uint8_t payload[max_payload]{};
};

// Encodes a frame; returns bytes written (0 if len > max_payload).
std::size_t encode(const Frame& f, std::uint8_t* out, std::size_t cap) noexcept;

// Byte-wise decoder; feed() returns true when a complete, CRC-valid frame is
// available in frame(). Bad CRC / oversize frames are dropped and counted.
class Decoder {
public:
    bool feed(std::uint8_t byte) noexcept;
    void reset() noexcept;
    const Frame& frame() const noexcept { return frame_; }
    std::uint32_t dropped() const noexcept { return dropped_; }
private:
    enum class S : std::uint8_t { Sync0, Sync1, Msg, Seq, Len, Payload, Crc0, Crc1 } s_ = S::Sync0;
    Frame frame_{};
    std::uint8_t idx_ = 0;
    std::uint16_t crc_rx_ = 0;
    std::uint32_t dropped_ = 0;
};

// Drops an incomplete frame after an idle interval without coupling the
// protocol parser to a particular transport. The Pico USB service calls
// expire() once per foreground iteration and passes its monotonic clock to
// feed(); host tests exercise the identical behavior.
class TimedDecoder {
public:
    explicit constexpr TimedDecoder(std::uint64_t timeout_us) : timeout_us_(timeout_us) {}
    bool feed(std::uint8_t byte, std::uint64_t now_us) noexcept {
        expire(now_us);
        last_rx_us_ = now_us;
        received_ = true;
        return decoder_.feed(byte);
    }
    void expire(std::uint64_t now_us) noexcept {
        if (received_ && now_us - last_rx_us_ > timeout_us_) {
            decoder_.reset();
            received_ = false;
        }
    }
    const Frame& frame() const noexcept { return decoder_.frame(); }
    std::uint32_t dropped() const noexcept { return decoder_.dropped(); }
private:
    Decoder decoder_{};
    std::uint64_t timeout_us_ = 0;
    std::uint64_t last_rx_us_ = 0;
    bool received_ = false;
};

// Payload helpers.
void put_u16(std::uint8_t* p, std::uint16_t v) noexcept;
void put_u32(std::uint8_t* p, std::uint32_t v) noexcept;
void put_u64(std::uint8_t* p, std::uint64_t v) noexcept;
std::uint16_t get_u16(const std::uint8_t* p) noexcept;
std::uint32_t get_u32(const std::uint8_t* p) noexcept;
std::uint64_t get_u64(const std::uint8_t* p) noexcept;
inline void put_i32(std::uint8_t* p, std::int32_t v) noexcept { put_u32(p, static_cast<std::uint32_t>(v)); }
inline std::int32_t get_i32(const std::uint8_t* p) noexcept { return static_cast<std::int32_t>(get_u32(p)); }
inline void put_i64(std::uint8_t* p, std::int64_t v) noexcept { put_u64(p, static_cast<std::uint64_t>(v)); }
inline std::int64_t get_i64(const std::uint8_t* p) noexcept { return static_cast<std::int64_t>(get_u64(p)); }

bool decode_job(const std::uint8_t* p, std::uint8_t len, Job& out) noexcept;
void encode_job(const Job& j, std::uint8_t* p) noexcept;
void encode_status(const Telemetry& t, std::uint8_t* p) noexcept;
// Rejects wrong-length payloads and non-canonical local telemetry encodings
// (unknown state/phase, reserved input/output bits, invalid job_loaded, or an
// invalid retained recipe). It does not validate physical machine conditions.
bool decode_status(const std::uint8_t* p, std::uint8_t len, Telemetry& t) noexcept;

// Applies a request frame to the controller and builds the reply frame.
void dispatch(Controller& c, const Frame& req, Frame& reply) noexcept;

const char* state_name(State s) noexcept;
const char* phase_name(Phase p) noexcept;
const char* result_name(CommandResult r) noexcept;
const char* fault_name(unsigned bit) noexcept;

} // namespace starz::fw::proto
