#pragma once
#include <algorithm>
#include "../protocol/command_validation.hpp"

namespace starz::hmi {

// Pure C++ status policy for deterministic stale/replay tests without a serial
// device or Qt event loop. Time is monotonic milliseconds supplied by the caller.
class TelemetryStatus {
public:
    static constexpr uint64_t timeout_ms = spec::heartbeat_timeout_us / 1000;

    void set_connected(bool connected) {
        connected_ = connected;
        sequences_ = {};
        invalidate();
    }
    void invalidate() { received_ = false; latest_ = {}; }
    bool fresh(uint64_t now_ms) const {
        return connected_ && received_ && now_ms >= received_at_ms_ && now_ms - received_at_ms_ < timeout_ms;
    }
    bool observe(const protocol::Packet& packet, uint64_t now_ms) {
        protocol::TelemetryPayload value{};
        if (!connected_ || packet.msg_id != static_cast<uint8_t>(protocol::MsgId::TELEM_STATUS) ||
            !protocol::decode_telemetry(packet.payload.data(), packet.len, value) ||
            !valid_reference_telemetry(value) || !sequences_.accept(packet.seq)) {
            invalidate();
            return false;
        }
        latest_ = value;
        received_at_ms_ = now_ms;
        received_ = true;
        return true;
    }
    const protocol::TelemetryPayload& latest() const { return latest_; }
    static bool valid_reference_telemetry(const protocol::TelemetryPayload& value) {
        const auto valid_position = [](uint8_t axis, int32_t position) {
            return position == protocol::UNKNOWN_POSITION_UM || protocol::position_in_range(axis, position);
        };
        return value.state <= static_cast<uint8_t>(protocol::MachineState::ESTOP) &&
               (value.safety_flags & protocol::REFERENCE_ONLY_FLAG) != 0 &&
               (value.error_code & 0xF000U) == 0 &&
               value.completed_pcs == 0 && value.target_pcs == 0 && value.cycle_time_ms == 0 &&
               valid_position(0, value.feed_command_um) && valid_position(0, value.encoder_meas_um) &&
               valid_position(2, value.blade_front_um) && valid_position(3, value.blade_rear_um);
    }
    static int progress_percent(uint32_t completed, uint32_t target) {
        if (target == 0) return 0;
        const uint64_t percent = static_cast<uint64_t>(completed) * 100U / target;
        return static_cast<int>(std::min<uint64_t>(percent, 100));
    }

private:
    bool connected_{false};
    bool received_{false};
    uint64_t received_at_ms_{0};
    protocol::TelemetryPayload latest_{};
    protocol::SequenceWindow sequences_;
};

} // namespace starz::hmi