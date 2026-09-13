#pragma once
#include "test_support.hpp"
#include "../firmware_controller.hpp"
#include <algorithm>

namespace starz::tests {

using protocol::CommandResult;
using protocol::Fault;
using protocol::MachineState;
using protocol::MsgId;

inline protocol::Packet packet(MsgId id, uint8_t sequence = 0) {
    protocol::Packet value;
    value.msg_id = static_cast<uint8_t>(id);
    value.seq = sequence;
    return value;
}

template<size_t Size>
protocol::Packet packet(MsgId id, uint8_t sequence, const std::array<uint8_t, Size>& payload) {
    static_assert(Size <= protocol::MAX_PAYLOAD_SIZE);
    auto value = packet(id, sequence);
    value.len = static_cast<uint8_t>(Size);
    std::copy(payload.begin(), payload.end(), value.payload.begin());
    return value;
}

inline CommandResult deliver(firmware::FirmwareController& controller, const protocol::Packet& value) {
    std::array<uint8_t, protocol::MAX_FRAME_SIZE> bytes{};
    const auto size = value.serialize(bytes.data(), bytes.size());
    CHECK(size != 0);
    for (size_t i = 0; i < size; ++i) controller.process_incoming_byte(bytes[i]);
    return controller.last_result();
}

inline bool has_fault(const firmware::FirmwareController& controller, Fault fault) {
    return (controller.faults() & protocol::fault_bit(fault)) != 0;
}

inline firmware::SafetyInputs healthy_inputs() { return {true, true, true, true, true, false, false}; }

inline std::array<firmware::AxisFeedback, protocol::AXIS_COUNT> measured_home_fixture() {
    std::array<firmware::AxisFeedback, protocol::AXIS_COUNT> feedback{};
    for (auto& axis : feedback) axis = {true, true, 0};
    return feedback;
}

struct ReferenceFixture {
    firmware::FirmwareController controller{firmware::ReferenceMode::FaultInjection};
    firmware::SafetyInputs inputs{healthy_inputs()};
    std::array<firmware::AxisFeedback, protocol::AXIS_COUNT> feedback{measured_home_fixture()};
    uint8_t next_sequence{0};

    ReferenceFixture() {
        sample();
        ping();
        CHECK(controller.trigger_recovery() == CommandResult::Accepted);
        CHECK(controller.get_state() == MachineState::IDLE);
        CHECK(!controller.reference_motion_active());
        CHECK(!controller.hardware_outputs_enabled());
    }
    void sample() {
        controller.inject_inputs_for_test(inputs);
        controller.inject_feedback_for_test(feedback);
    }
    void ping() { CHECK(deliver(controller, packet(MsgId::CMD_PING, next_sequence++)) == CommandResult::Accepted); }
    void advance(uint32_t delta_us, bool follow = false, bool heartbeat = true) {
        // This deliberately labeled ideal fixture is OUTSIDE the controller.
        // For stuck-feedback tests, leave the observation unchanged (follow=false).
        if (follow) {
            for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
                const auto position = controller.reference_command_um(axis);
                if (position != protocol::UNKNOWN_POSITION_UM) feedback[axis].position_um = position;
            }
        }
        sample();
        if (heartbeat) ping();
        controller.update_tick(delta_us);
        CHECK(controller.get_telemetry().completed_pcs == 0);
        CHECK(controller.get_telemetry().target_pcs == 0);
        CHECK(controller.get_telemetry().cycle_time_ms == 0);
        CHECK(!controller.hardware_outputs_enabled());
    }
};

} // namespace starz::tests