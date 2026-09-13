#include "safety_controller.hpp"

namespace starz::firmware {

SafetyController::SafetyController() = default;

void SafetyController::init() {
    inputs_ = {};
    input_age_us_ = protocol::INPUT_TIMEOUT_US;
    unlock_requested_ = false;
    trip(protocol::Fault::InputsUnknown);
}

void SafetyController::update_inputs(bool safety_relay_in, bool guard_switch_in, bool wire_sensor_in, bool slug_sensor_in, bool driver_alarm_in) {
    update_inputs({true, safety_relay_in, guard_switch_in, wire_sensor_in, slug_sensor_in, driver_alarm_in, false});
}

void SafetyController::update_inputs(const SafetyInputs& sample) {
    inputs_ = sample;
    input_age_us_ = 0;
    // Level-triggered and immediate: no debounce, edge qualification or delay.
    if (!sample.valid) {
        trip(protocol::Fault::InputsUnknown);
    } else {
        if (!sample.safety_relay_ok) trip(protocol::Fault::Estop);
        if (!sample.guard_locked) trip(protocol::Fault::GuardOpen);
        if (sample.driver_alarm) trip(protocol::Fault::DriverAlarm);
        if (!sample.wire_present) trip(protocol::Fault::WireMissing);
        if (!sample.slug_clear) trip(protocol::Fault::SlugJam);
    }
    if (!sample.valid || !sample.independent_unlock_permission) unlock_requested_ = false;
}

void SafetyController::elapse(uint64_t delta_us) {
    input_age_us_ = protocol::age_by(input_age_us_, delta_us, protocol::INPUT_TIMEOUT_US);
    if (inputs_.valid && !inputs_fresh()) trip(protocol::Fault::InputsStale);
    if (!inputs_fresh()) unlock_requested_ = false;
}

bool SafetyController::inputs_fresh() const {
    return inputs_.valid && input_age_us_ < protocol::INPUT_TIMEOUT_US;
}

bool SafetyController::healthy_observation() const {
    return inputs_fresh() && inputs_.safety_relay_ok && inputs_.guard_locked &&
           inputs_.wire_present && inputs_.slug_clear && !inputs_.driver_alarm;
}

bool SafetyController::is_safe_to_move() const {
    return healthy_observation() && faults_ == 0 && !unlock_requested_;
}

bool SafetyController::deliberate_recovery() {
    if (!healthy_observation() || unlock_requested_) return false;
    faults_ = 0;
    return true;
}

bool SafetyController::request_door_unlock() {
    // Software cannot infer permission from idle, zero speed or elapsed time.
    // This input represents an INDEPENDENT permission in the test fixture only.
    if (!inputs_fresh() || !inputs_.independent_unlock_permission) return false;
    unlock_requested_ = true;
    return true;
}

void SafetyController::lock_door() {
    unlock_requested_ = false; // A request only; measured guard feedback is untouched.
}

uint8_t SafetyController::get_flags_byte() const {
    uint8_t flags = protocol::REFERENCE_ONLY_FLAG;
    if (inputs_fresh()) {
        flags |= protocol::INPUTS_FRESH_FLAG;
        if (inputs_.safety_relay_ok) flags |= 0x01;
        if (inputs_.guard_locked) flags |= 0x02;
        if (inputs_.wire_present) flags |= 0x04;
        if (inputs_.slug_clear) flags |= 0x08;
        if (inputs_.driver_alarm) flags |= 0x10;
    }
    if ((faults_ & protocol::fault_bit(protocol::Fault::Estop)) != 0) flags |= 0x20;
    return flags;
}

protocol::MachineState SafetyController::evaluate_state(protocol::MachineState current_state) const {
    if ((faults_ & protocol::fault_bit(protocol::Fault::Estop)) != 0) {
        return protocol::MachineState::ESTOP;
    }
    if (faults_ != 0 || !healthy_observation()) return protocol::MachineState::FAULT;
    return current_state;
}

} // namespace starz::firmware
