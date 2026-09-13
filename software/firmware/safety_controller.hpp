#pragma once
#include <cstdint>
#include "reference_only.hpp"
#include "../protocol/command_validation.hpp"

namespace starz::firmware {

// A copied observation, never a commanded state. No GPIO or independent safety
// circuitry is implemented here. Unknown/absent/stale observations deny motion.
struct SafetyInputs {
    bool valid{false};
    bool safety_relay_ok{false};
    bool guard_locked{false};
    bool wire_present{false};
    bool slug_clear{false};
    bool driver_alarm{true};
    bool independent_unlock_permission{false};
};

class SafetyController {
public:
    SafetyController();

    // Reinitialization invalidates observations; it cannot clear a latched trip.
    void init();
    void update_inputs(bool safety_relay_in, bool guard_switch_in, bool wire_sensor_in, bool slug_sensor_in, bool driver_alarm_in);
    void update_inputs(const SafetyInputs& sample);
    void elapse(uint64_t delta_us);

    bool is_safe_to_move() const;
    bool inputs_fresh() const;
    bool healthy_observation() const;
    bool deliberate_recovery();
    bool request_door_unlock();
    void lock_door();
    bool unlock_requested() const { return unlock_requested_; }
    const SafetyInputs& measured_inputs() const { return inputs_; }
    uint16_t faults() const { return faults_; }

    uint8_t get_flags_byte() const;
    protocol::MachineState evaluate_state(protocol::MachineState current_state) const;

private:
    void trip(protocol::Fault fault) { faults_ = static_cast<uint16_t>(faults_ | protocol::fault_bit(fault)); }
    SafetyInputs inputs_{};
    uint64_t input_age_us_{protocol::INPUT_TIMEOUT_US};
    uint16_t faults_{protocol::fault_bit(protocol::Fault::InputsUnknown)};
    bool unlock_requested_{false};
};

} // namespace starz::firmware
