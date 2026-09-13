#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <bit>
#include <cstring>
#include <limits>

namespace starz::protocol {

inline constexpr uint16_t SOF_MAGIC = 0xA55A;
inline constexpr uint8_t MAX_PAYLOAD_SIZE = 64;
inline constexpr size_t MAX_FRAME_SIZE = 7U + MAX_PAYLOAD_SIZE;
inline constexpr uint8_t JOB_PAYLOAD_SIZE = 20;
inline constexpr uint8_t JOG_PAYLOAD_SIZE = 10;
inline constexpr uint8_t TELEMETRY_PAYLOAD_SIZE = 30;
inline constexpr int32_t UNKNOWN_POSITION_UM = std::numeric_limits<int32_t>::min();
inline constexpr uint8_t INPUTS_FRESH_FLAG = 0x40;
inline constexpr uint8_t REFERENCE_ONLY_FLAG = 0x80;

// Wire contract: SOF and CRC are MSB first; *payload* integers are little endian,
// signed integers are two's complement. Never memcpy a C++ struct onto the wire.

// Message Identifiers
enum class MsgId : uint8_t {
    CMD_PING              = 0x01,
    RESP_PONG             = 0x02,
    CMD_JOB_START         = 0x10,
    CMD_JOB_PAUSE         = 0x11,
    CMD_JOB_RESUME        = 0x12,
    CMD_JOB_ABORT         = 0x13,
    CMD_RECOVER           = 0x14, // Deliberate fault acknowledgement; never starts motion.
    CMD_JOG_AXIS          = 0x20,
    CMD_HOME_ALL          = 0x21,
    CMD_SOLENOID_AIR      = 0x22,
    CMD_SOLENOID_DOOR     = 0x23,
    CMD_SET_CALIB         = 0x30,
    TELEM_STATUS          = 0x80,
    RESP_ACK              = 0xFE,
    RESP_NACK             = 0xFF
};

// Machine State Enum
enum class MachineState : uint8_t {
    INIT     = 0,
    IDLE     = 1,
    HOMING   = 2,
    RUNNING  = 3,
    PAUSED   = 4,
    FAULT    = 5,
    ESTOP    = 6
};

enum class CommandResult : uint8_t {
    Accepted = 0,
    InvalidLength,
    InvalidArgument,
    UnsafeState,
    NotCommissioned,
    HomeUnknown,
    OutOfRange,
    Busy,
    Replay,
    Unsupported,
    PermissionDenied,
    MalformedFrame
};

// Sticky diagnostic bits. These are software observations, not certified safety.
enum class Fault : uint16_t {
    InputsUnknown = 1U << 0,
    InputsStale = 1U << 1,
    Estop = 1U << 2,
    GuardOpen = 1U << 3,
    DriverAlarm = 1U << 4,
    WireMissing = 1U << 5,
    SlugJam = 1U << 6,
    HeartbeatLost = 1U << 7,
    FeedbackUnknown = 1U << 8,
    FeedbackStale = 1U << 9,
    FollowingError = 1U << 10,
    MoveTimeout = 1U << 11
};

constexpr uint16_t fault_bit(Fault fault) { return static_cast<uint16_t>(fault); }

// Safety Status Bitfield
struct SafetyFlags {
    uint8_t safety_relay_ok    : 1;
    uint8_t guard_door_locked  : 1;
    uint8_t wire_inlet_present : 1;
    uint8_t slug_jam_clear     : 1;
    uint8_t motor_driver_alarm : 1;
    uint8_t emergency_stop     : 1;
    uint8_t inputs_fresh       : 1;
    uint8_t reference_only     : 1;
};

// Payload for CMD_JOB_START
#pragma pack(push, 1)
struct JobStartPayload {
    int32_t wire_length_um;       // Wire cut length in micrometres (e.g. 1000000 um = 1.0 m)
    int32_t lead_strip_um;        // Front strip length (e.g. 10000 um = 10.0 mm)
    int32_t tail_strip_um;        // Rear strip length (e.g. 10000 um = 10.0 mm)
    int32_t blade_strip_depth_um; // Cutting jaw penetration depth (e.g. 3000 um)
    uint32_t target_quantity;     // Number of wires to process
};

// Payload for CMD_JOG_AXIS
struct JogAxisPayload {
    uint8_t axis_id;              // 0=Feed L, 1=Feed R, 2=Cut F, 3=Cut R
    int8_t direction;             // +1 or -1
    int32_t speed_um_s;           // Speed in micrometres per second
    int32_t distance_um;          // Distance in micrometres
};

// Payload for TELEM_STATUS
struct TelemetryPayload {
    uint8_t state;                // MachineState
    uint8_t safety_flags;         // Bitfield
    int32_t feed_command_um;      // Commanded feed position
    int32_t encoder_meas_um;      // Injected reference feedback; INT32_MIN means unknown
    int32_t blade_front_um;       // Front jaw position
    int32_t blade_rear_um;        // Rear jaw position
    uint32_t completed_pcs;       // Always zero in this uncommissioned reference
    uint32_t target_pcs;          // Target batch quantity
    uint16_t cycle_time_ms;       // Last cycle duration in ms
    uint16_t error_code;          // Sticky Fault bitmask; zero is NOT a safety approval
};
#pragma pack(pop)

static_assert(sizeof(JobStartPayload) == JOB_PAYLOAD_SIZE);
static_assert(sizeof(JogAxisPayload) == JOG_PAYLOAD_SIZE);
static_assert(sizeof(TelemetryPayload) == TELEMETRY_PAYLOAD_SIZE);

inline void put_u16_le(uint8_t* out, uint16_t value) {
    out[0] = static_cast<uint8_t>(value);
    out[1] = static_cast<uint8_t>(value >> 8);
}
inline void put_u32_le(uint8_t* out, uint32_t value) {
    for (size_t i = 0; i < 4; ++i) out[i] = static_cast<uint8_t>(value >> (8U * i));
}
inline void put_i32_le(uint8_t* out, int32_t value) {
    put_u32_le(out, std::bit_cast<uint32_t>(value));
}
inline uint16_t get_u16_le(const uint8_t* in) {
    return static_cast<uint16_t>(static_cast<uint16_t>(in[0]) |
                                 (static_cast<uint16_t>(in[1]) << 8));
}
inline uint32_t get_u32_le(const uint8_t* in) {
    uint32_t value = 0;
    for (size_t i = 0; i < 4; ++i) value |= static_cast<uint32_t>(in[i]) << (8U * i);
    return value;
}
inline int32_t get_i32_le(const uint8_t* in) {
    return std::bit_cast<int32_t>(get_u32_le(in));
}

inline std::array<uint8_t, JOB_PAYLOAD_SIZE> encode_job(const JobStartPayload& value) {
    std::array<uint8_t, JOB_PAYLOAD_SIZE> out{};
    put_i32_le(out.data(), value.wire_length_um);
    put_i32_le(out.data() + 4, value.lead_strip_um);
    put_i32_le(out.data() + 8, value.tail_strip_um);
    put_i32_le(out.data() + 12, value.blade_strip_depth_um);
    put_u32_le(out.data() + 16, value.target_quantity);
    return out;
}
inline bool decode_job(const uint8_t* in, size_t len, JobStartPayload& value) {
    if (in == nullptr || len != JOB_PAYLOAD_SIZE) return false;
    value = {get_i32_le(in), get_i32_le(in + 4), get_i32_le(in + 8),
             get_i32_le(in + 12), get_u32_le(in + 16)};
    return true;
}
inline std::array<uint8_t, JOG_PAYLOAD_SIZE> encode_jog(const JogAxisPayload& value) {
    std::array<uint8_t, JOG_PAYLOAD_SIZE> out{};
    out[0] = value.axis_id;
    out[1] = std::bit_cast<uint8_t>(value.direction);
    put_i32_le(out.data() + 2, value.speed_um_s);
    put_i32_le(out.data() + 6, value.distance_um);
    return out;
}
inline bool decode_jog(const uint8_t* in, size_t len, JogAxisPayload& value) {
    if (in == nullptr || len != JOG_PAYLOAD_SIZE) return false;
    value = {in[0], std::bit_cast<int8_t>(in[1]), get_i32_le(in + 2), get_i32_le(in + 6)};
    return true;
}
inline std::array<uint8_t, TELEMETRY_PAYLOAD_SIZE> encode_telemetry(const TelemetryPayload& value) {
    std::array<uint8_t, TELEMETRY_PAYLOAD_SIZE> out{};
    out[0] = value.state;
    out[1] = value.safety_flags;
    put_i32_le(out.data() + 2, value.feed_command_um);
    put_i32_le(out.data() + 6, value.encoder_meas_um);
    put_i32_le(out.data() + 10, value.blade_front_um);
    put_i32_le(out.data() + 14, value.blade_rear_um);
    put_u32_le(out.data() + 18, value.completed_pcs);
    put_u32_le(out.data() + 22, value.target_pcs);
    put_u16_le(out.data() + 26, value.cycle_time_ms);
    put_u16_le(out.data() + 28, value.error_code);
    return out;
}
inline bool decode_telemetry(const uint8_t* in, size_t len, TelemetryPayload& value) {
    if (in == nullptr || len != TELEMETRY_PAYLOAD_SIZE) return false;
    value = {in[0], in[1], get_i32_le(in + 2), get_i32_le(in + 6), get_i32_le(in + 10),
             get_i32_le(in + 14), get_u32_le(in + 18), get_u32_le(in + 22),
             get_u16_le(in + 26), get_u16_le(in + 28)};
    return true;
}

// Standard CRC-16-CCITT implementation (polynomial 0x1021, init 0xFFFF)
inline uint16_t compute_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc = static_cast<uint16_t>(crc ^ (static_cast<uint16_t>(data[i]) << 8));
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000) {
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
            } else {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
    }
    return crc;
}

// Framing container
struct Packet {
    uint16_t sof{SOF_MAGIC};
    uint8_t msg_id{0};
    uint8_t seq{0};
    uint8_t len{0};
    std::array<uint8_t, MAX_PAYLOAD_SIZE> payload{};
    uint16_t crc{0};

    // Serialize packet into buffer, returning total bytes written
    size_t serialize(uint8_t* out_buffer, size_t max_out) const {
        const size_t frame_size = 7U + static_cast<size_t>(len);
        if (len > MAX_PAYLOAD_SIZE || out_buffer == nullptr || max_out < frame_size) return 0;
        out_buffer[0] = static_cast<uint8_t>((SOF_MAGIC >> 8) & 0xFF);
        out_buffer[1] = static_cast<uint8_t>(SOF_MAGIC & 0xFF);
        out_buffer[2] = msg_id;
        out_buffer[3] = seq;
        out_buffer[4] = len;
        if (len > 0) {
            std::memcpy(&out_buffer[5], payload.data(), len);
        }
        uint16_t calculated_crc = compute_crc16(&out_buffer[2], 3 + len);
        out_buffer[5 + len] = static_cast<uint8_t>((calculated_crc >> 8) & 0xFF);
        out_buffer[6 + len] = static_cast<uint8_t>(calculated_crc & 0xFF);
        return frame_size;
    }
};

enum class ParseResult { Incomplete, Complete, InvalidLength, BadCrc };

// Fixed storage. Partial frames expire in the caller; CRC is corruption detection,
// NOT authentication. An invalid frame never reaches command dispatch.
class PacketParser {
public:
    void reset() { state_ = 0; index_ = 0; packet_ = {}; }
    bool partial() const { return state_ != 0; }

    ParseResult consume(uint8_t byte, Packet& out) {
        switch (state_) {
        case 0:
            if (byte == 0xA5) state_ = 1;
            break;
        case 1:
            if (byte == 0x5A) state_ = 2;
            else state_ = byte == 0xA5 ? 1 : 0;
            break;
        case 2: packet_.msg_id = byte; state_ = 3; break;
        case 3: packet_.seq = byte; state_ = 4; break;
        case 4:
            packet_.len = byte;
            index_ = 0;
            if (byte > MAX_PAYLOAD_SIZE) {
                reset();
                return ParseResult::InvalidLength;
            }
            state_ = byte == 0 ? 6 : 5;
            break;
        case 5:
            packet_.payload[index_++] = byte;
            if (index_ == packet_.len) state_ = 6;
            break;
        case 6:
            packet_.crc = static_cast<uint16_t>(static_cast<uint16_t>(byte) << 8);
            state_ = 7;
            break;
        case 7: {
            packet_.crc = static_cast<uint16_t>(packet_.crc | byte);
            std::array<uint8_t, 3U + MAX_PAYLOAD_SIZE> check{};
            check[0] = packet_.msg_id;
            check[1] = packet_.seq;
            check[2] = packet_.len;
            std::memcpy(check.data() + 3, packet_.payload.data(), packet_.len);
            const bool valid = compute_crc16(check.data(), 3U + packet_.len) == packet_.crc;
            if (valid) out = packet_;
            reset();
            return valid ? ParseResult::Complete : ParseResult::BadCrc;
        }
        default: reset(); break;
        }
        return ParseResult::Incomplete;
    }

private:
    uint8_t state_{0};
    size_t index_{0};
    Packet packet_{};
};

// Limited duplicate/out-of-order filtering within an 8-bit half-range window.
// This cannot prevent replay after wrap, reboot or reconnection; it is not security.
class SequenceWindow {
public:
    bool accept(uint8_t sequence) {
        const auto distance = static_cast<uint8_t>(sequence - last_);
        if (seen_ && (distance == 0 || distance >= 128)) return false;
        seen_ = true;
        last_ = sequence;
        return true;
    }
private:
    bool seen_{false};
    uint8_t last_{0};
};

} // namespace starz::protocol
