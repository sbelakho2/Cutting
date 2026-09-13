#include "protocol.hpp"
#include "crc16.hpp"
#include <cstring>

namespace starz::fw::proto {

void put_u16(std::uint8_t* p, std::uint16_t v) noexcept { p[0] = static_cast<std::uint8_t>(v); p[1] = static_cast<std::uint8_t>(v >> 8); }
void put_u32(std::uint8_t* p, std::uint32_t v) noexcept {
    p[0] = static_cast<std::uint8_t>(v); p[1] = static_cast<std::uint8_t>(v >> 8);
    p[2] = static_cast<std::uint8_t>(v >> 16); p[3] = static_cast<std::uint8_t>(v >> 24);
}
void put_u64(std::uint8_t* p, std::uint64_t v) noexcept {
    for (std::uint8_t i = 0; i < 8; ++i) p[i] = static_cast<std::uint8_t>(v >> (8u * i));
}
std::uint16_t get_u16(const std::uint8_t* p) noexcept { return static_cast<std::uint16_t>(p[0] | (p[1] << 8)); }
std::uint32_t get_u32(const std::uint8_t* p) noexcept {
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24);
}
std::uint64_t get_u64(const std::uint8_t* p) noexcept {
    std::uint64_t v = 0;
    for (std::uint8_t i = 0; i < 8; ++i) v |= static_cast<std::uint64_t>(p[i]) << (8u * i);
    return v;
}

std::size_t encode(const Frame& f, std::uint8_t* out, std::size_t cap) noexcept {
    if (f.len > max_payload || cap < 5u + f.len + 2u) return 0;
    out[0] = sync0; out[1] = sync1;
    out[2] = static_cast<std::uint8_t>(f.msg); out[3] = f.seq; out[4] = f.len;
    std::memcpy(out + 5, f.payload, f.len);
    const std::uint16_t crc = crc16(out + 2, 3u + f.len);
    out[5 + f.len] = static_cast<std::uint8_t>(crc >> 8);
    out[6 + f.len] = static_cast<std::uint8_t>(crc);
    return 7u + f.len;
}

bool Decoder::feed(std::uint8_t b) noexcept {
    switch (s_) {
    case S::Sync0: if (b == sync0) s_ = S::Sync1; return false;
    case S::Sync1: s_ = (b == sync1) ? S::Msg : (b == sync0 ? S::Sync1 : S::Sync0); return false;
    case S::Msg: frame_.msg = static_cast<Msg>(b); s_ = S::Seq; return false;
    case S::Seq: frame_.seq = b; s_ = S::Len; return false;
    case S::Len:
        if (b > max_payload) { ++dropped_; s_ = S::Sync0; return false; }
        frame_.len = b; idx_ = 0; s_ = b ? S::Payload : S::Crc0; return false;
    case S::Payload:
        frame_.payload[idx_++] = b;
        if (idx_ >= frame_.len) s_ = S::Crc0;
        return false;
    case S::Crc0: crc_rx_ = static_cast<std::uint16_t>(b << 8); s_ = S::Crc1; return false;
    case S::Crc1: {
        crc_rx_ = static_cast<std::uint16_t>(crc_rx_ | b);
        s_ = S::Sync0;
        std::uint8_t hdr[3 + max_payload];
        hdr[0] = static_cast<std::uint8_t>(frame_.msg); hdr[1] = frame_.seq; hdr[2] = frame_.len;
        std::memcpy(hdr + 3, frame_.payload, frame_.len);
        if (crc16(hdr, 3u + frame_.len) != crc_rx_) { ++dropped_; return false; }
        return true;
    }
    }
    return false;
}

void Decoder::reset() noexcept {
    s_ = S::Sync0;
    idx_ = 0;
    crc_rx_ = 0;
}

bool decode_job(const std::uint8_t* p, std::uint8_t len, Job& j) noexcept {
    if (len != load_job_len) return false;
    j.length_um = get_i32(p); j.strip_lead_um = get_i32(p + 4); j.strip_tail_um = get_i32(p + 8);
    j.strip_depth_um = get_i32(p + 12); j.wire_diameter_um = get_i32(p + 16); j.quantity = get_u32(p + 20);
    return true;
}
void encode_job(const Job& j, std::uint8_t* p) noexcept {
    put_i32(p, j.length_um); put_i32(p + 4, j.strip_lead_um); put_i32(p + 8, j.strip_tail_um);
    put_i32(p + 12, j.strip_depth_um); put_i32(p + 16, j.wire_diameter_um); put_u32(p + 20, j.quantity);
}

namespace {
std::uint16_t pack_inputs(const Inputs& i) {
    std::uint16_t v = 0;
    v |= i.start ? 1u : 0u; v |= i.stop_ok ? 2u : 0u; v |= i.reset ? 4u : 0u;
    v |= i.safety_ok ? 8u : 0u; v |= i.home_front ? 16u : 0u; v |= i.home_rear ? 32u : 0u;
    for (unsigned k = 0; k < 4; ++k) if (i.alm[k]) v = static_cast<std::uint16_t>(v | (1u << (8 + k)));
    return v;
}
void unpack_inputs(std::uint16_t v, Inputs& i) {
    i.start = v & 1u; i.stop_ok = v & 2u; i.reset = v & 4u; i.safety_ok = v & 8u;
    i.home_front = v & 16u; i.home_rear = v & 32u;
    for (unsigned k = 0; k < 4; ++k) i.alm[k] = (v >> (8 + k)) & 1u;
}

inline constexpr std::uint16_t status_input_mask = 0x0f3fu;
inline constexpr std::uint8_t status_output_mask = 0x0fu;
}

void encode_status(const Telemetry& t, std::uint8_t* p) noexcept {
    p[0] = static_cast<std::uint8_t>(t.state); p[1] = static_cast<std::uint8_t>(t.phase);
    put_u16(p + 2, t.faults); put_u16(p + 4, pack_inputs(t.inputs));
    p[6] = static_cast<std::uint8_t>((t.outputs.led_green ? 1 : 0) | (t.outputs.led_red ? 2 : 0) |
                                     (t.outputs.buzzer ? 4 : 0) | (t.outputs.drives_disabled ? 8 : 0));
    p[7] = t.job_loaded ? 1 : 0;
    put_u32(p + 8, t.counters.pieces_done); put_u32(p + 12, t.counters.pieces_scrap);
    put_i32(p + 16, t.counters.last_residual_um); put_i32(p + 20, t.counters.worst_residual_um);
    put_i32(p + 24, t.blade_um[0]); put_i32(p + 28, t.blade_um[1]);
    put_i64(p + 32, t.feed_um[0]); put_i64(p + 40, t.feed_um[1]);
    put_i64(p + 48, t.encoder_counts[0]); put_i64(p + 56, t.encoder_counts[1]);
    put_i32(p + 64, t.settings.counts_per_metre[0]); put_i32(p + 68, t.settings.counts_per_metre[1]);
    put_u32(p + 72, static_cast<std::uint32_t>(t.uptime_us / 1000));
    encode_job(t.job, p + 76);
}

bool decode_status(const std::uint8_t* p, std::uint8_t len, Telemetry& t) noexcept {
    if (p == nullptr || len != status_len) return false;
    const std::uint16_t input_bits = get_u16(p + 4);
    if (p[0] > static_cast<std::uint8_t>(State::Jogging) ||
        p[1] > static_cast<std::uint8_t>(Phase::Stop) ||
        (input_bits & static_cast<std::uint16_t>(~status_input_mask)) != 0 ||
        (p[6] & static_cast<std::uint8_t>(~status_output_mask)) != 0 || p[7] > 1) return false;

    Telemetry decoded{};
    decoded.state = static_cast<State>(p[0]); decoded.phase = static_cast<Phase>(p[1]);
    decoded.faults = get_u16(p + 2); unpack_inputs(input_bits, decoded.inputs);
    decoded.outputs.led_green = p[6] & 1; decoded.outputs.led_red = p[6] & 2;
    decoded.outputs.buzzer = p[6] & 4; decoded.outputs.drives_disabled = p[6] & 8;
    decoded.job_loaded = p[7] == 1;
    decoded.counters.pieces_done = get_u32(p + 8); decoded.counters.pieces_scrap = get_u32(p + 12);
    decoded.counters.last_residual_um = get_i32(p + 16); decoded.counters.worst_residual_um = get_i32(p + 20);
    decoded.blade_um[0] = get_i32(p + 24); decoded.blade_um[1] = get_i32(p + 28);
    decoded.feed_um[0] = get_i64(p + 32); decoded.feed_um[1] = get_i64(p + 40);
    decoded.encoder_counts[0] = get_i64(p + 48); decoded.encoder_counts[1] = get_i64(p + 56);
    decoded.settings.counts_per_metre[0] = get_i32(p + 64); decoded.settings.counts_per_metre[1] = get_i32(p + 68);
    decoded.uptime_us = static_cast<std::uint64_t>(get_u32(p + 72)) * 1000ull;
    if (!decode_job(p + 76, load_job_len, decoded.job) ||
        (decoded.job_loaded && validate(decoded.job) != JobError::None)) return false;
    decoded.settings.stored_job = decoded.job;
    decoded.settings.stored_job_valid = decoded.job_loaded ? Settings::stored_job_marker : 0;
    t = decoded;
    return true;
}

void dispatch(Controller& c, const Frame& req, Frame& reply) noexcept {
    reply = Frame{};
    reply.seq = req.seq;
    auto ack = [&](CommandResult r) {
        reply.msg = r == CommandResult::Accepted ? Msg::Ack : Msg::Nack;
        reply.len = 1; reply.payload[0] = static_cast<std::uint8_t>(r);
    };
    auto require_empty = [&]() { return req.len == 0; };
    switch (req.msg) {
    case Msg::Hello: {
        if (!require_empty()) { ack(CommandResult::MalformedFrame); return; }
        reply.msg = Msg::HelloAck;
        reply.payload[0] = protocol_version;
        const std::size_t n = std::strlen(ctrl::revision);
        std::memcpy(reply.payload + 1, ctrl::revision, n + 1);
        reply.len = static_cast<std::uint8_t>(n + 2);
        return;
    }
    case Msg::GetStatus:
        if (!require_empty()) { ack(CommandResult::MalformedFrame); return; }
        reply.msg = Msg::Status; reply.len = status_len;
        encode_status(c.telemetry(), reply.payload);
        return;
    case Msg::LoadJob: {
        Job j{};
        if (!decode_job(req.payload, req.len, j)) { ack(CommandResult::MalformedFrame); return; }
        ack(c.load_job(j)); return;
    }
    case Msg::ClearJob: if (!require_empty()) { ack(CommandResult::MalformedFrame); return; } ack(c.clear_job()); return;
    case Msg::Start: if (!require_empty()) { ack(CommandResult::MalformedFrame); return; } ack(c.start()); return;
    case Msg::Stop: if (!require_empty()) { ack(CommandResult::MalformedFrame); return; } ack(c.stop()); return;
    case Msg::Reset: if (!require_empty()) { ack(CommandResult::MalformedFrame); return; } ack(c.reset()); return;
    case Msg::Home: if (!require_empty()) { ack(CommandResult::MalformedFrame); return; } ack(c.home()); return;
    case Msg::Jog:
        if (req.len != jog_len) { ack(CommandResult::MalformedFrame); return; }
        if (req.payload[0] >= ctrl::axis::count || req.payload[1] > 1) { ack(CommandResult::InvalidArgument); return; }
        ack(c.jog(static_cast<Axis>(req.payload[0]), req.payload[1] != 0, get_i32(req.payload + 2), get_i32(req.payload + 6)));
        return;
    case Msg::SetCalibration:
        if (req.len != calibration_len) { ack(CommandResult::MalformedFrame); return; }
        ack(c.set_calibration(get_i32(req.payload), get_i32(req.payload + 4)));
        return;
    default: ack(CommandResult::Unsupported); return;
    }
}

const char* state_name(State s) noexcept {
    switch (s) {
    case State::PowerOn: return "PowerOn"; case State::Fault: return "Fault"; case State::Unhomed: return "Unhomed";
    case State::Homing: return "Homing"; case State::Ready: return "Ready"; case State::Running: return "Running";
    case State::Stopping: return "Stopping"; case State::Paused: return "Paused"; case State::Jogging: return "Jogging";
    }
    return "?";
}
const char* phase_name(Phase p) noexcept {
    switch (p) {
    case Phase::Idle: return "Idle"; case Phase::HomeLeave: return "HomeLeave"; case Phase::HomeSeek: return "HomeSeek";
    case Phase::HomeBackoff: return "HomeBackoff"; case Phase::HomeApproach: return "HomeApproach";
    case Phase::EnableSettle: return "EnableSettle"; case Phase::Feed: return "Feed"; case Phase::FeedVerify: return "FeedVerify";
    case Phase::FeedCorrect: return "FeedCorrect"; case Phase::Cut: return "Cut"; case Phase::OpenAfterCut: return "OpenAfterCut";
    case Phase::StripPosition: return "StripPosition"; case Phase::StripClose: return "StripClose"; case Phase::StripPull: return "StripPull";
    case Phase::Open: return "Open"; case Phase::Eject: return "Eject"; case Phase::PieceDone: return "PieceDone";
    case Phase::Jog: return "Jog"; case Phase::Stop: return "Stop";
    }
    return "?";
}
const char* result_name(CommandResult r) noexcept {
    switch (r) {
    case CommandResult::Accepted: return "Accepted"; case CommandResult::InvalidArgument: return "InvalidArgument";
    case CommandResult::WrongState: return "WrongState"; case CommandResult::Busy: return "Busy";
    case CommandResult::Faulted: return "Faulted"; case CommandResult::NotHomed: return "NotHomed";
    case CommandResult::NoJob: return "NoJob"; case CommandResult::Unsupported: return "Unsupported";
    case CommandResult::MalformedFrame: return "MalformedFrame"; case CommandResult::StorageError: return "StorageError";
    }
    return "?";
}
const char* fault_name(unsigned bit) noexcept {
    switch (bit) {
    case 0: return "alm_blade_front"; case 1: return "alm_blade_rear"; case 2: return "alm_feed_left"; case 3: return "alm_feed_right";
    case 4: return "safety_open"; case 5: return "stop_pressed"; case 6: return "homing_failed"; case 7: return "move_timeout";
    case 8: return "length_slip"; case 9: return "no_wire"; case 10: return "blade_limit"; case 11: return "encoder_stalled";
    case 12: return "encoder_overflow"; case 13: return "watchdog_reset"; case 14: return "encoder_invalid";
    case 15: return "internal";
    }
    return "reserved";
}

} // namespace starz::fw::proto
