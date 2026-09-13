#pragma once
#include <array>
#include <cstdint>
#include "motion_planner.hpp"
#include "safety_controller.hpp"

namespace starz::firmware {

enum class ReferenceMode { MonitorOnly, FaultInjection };

struct AxisFeedback {
    bool valid{false};
    bool measured_home_known{false};
    int32_t position_um{protocol::UNKNOWN_POSITION_UM};
};

// Host-only, single-threaded, caller-clocked reference. No process model, real
// encoders, PIO, hardware watchdog, drive enable or certified safety function.
class FirmwareController {
public:
    explicit FirmwareController(ReferenceMode mode = ReferenceMode::MonitorOnly);

    void init();
    void process_incoming_byte(uint8_t byte);
    void update_tick(uint32_t delta_us);
    bool take_response(protocol::Packet& packet);

    protocol::TelemetryPayload get_telemetry() const;
    protocol::MachineState get_state() const { return current_state_; }
    protocol::CommandResult last_result() const { return last_result_; }
    uint16_t faults() const { return static_cast<uint16_t>(monitor_faults_ | safety_.faults()); }
    bool hardware_outputs_enabled() const { return false; }
    bool reference_motion_active() const { return reference_motion_enabled_; }
    int32_t reference_command_um(uint8_t axis) const { return motion_.get_axis_pos_um(axis); }
    const SafetyInputs& measured_inputs() const { return safety_.measured_inputs(); }
    bool unlock_requested() const { return safety_.unlock_requested(); }

    protocol::CommandResult trigger_job_start(int32_t wire_length_um, int32_t lead_strip_um, int32_t tail_strip_um, int32_t blade_depth_um, uint32_t qty);
    protocol::CommandResult trigger_jog(uint8_t axis_id, int8_t direction, int32_t speed_um_s, int32_t distance_um);
    protocol::CommandResult trigger_home();
    protocol::CommandResult trigger_abort();
    protocol::CommandResult trigger_pause();
    protocol::CommandResult trigger_resume();
    protocol::CommandResult trigger_recovery();
    protocol::CommandResult request_door_unlock();
    void request_door_lock() { safety_.lock_door(); }

    // Explicit test seams only. Samples are copied; commands never modify them.
    void inject_inputs_for_test(const SafetyInputs& inputs);
    void inject_feedback_for_test(const std::array<AxisFeedback, protocol::AXIS_COUNT>& feedback);

private:
    void handle_packet(const protocol::Packet& pkt);
    protocol::CommandResult dispatch_command(const protocol::Packet& pkt);
    protocol::CommandResult remember(protocol::CommandResult result) { last_result_ = result; return result; }
    void latch(protocol::Fault fault) { monitor_faults_ = static_cast<uint16_t>(monitor_faults_ | protocol::fault_bit(fault)); }
    void enforce_faults();
    void monitor_feedback();
    bool feedback_ready() const;
    bool heartbeat_fresh() const;
    void stop_reference_motion();

    ReferenceMode mode_;
    protocol::MachineState current_state_{protocol::MachineState::FAULT};
    protocol::CommandResult last_result_{protocol::CommandResult::UnsafeState};

    MotionPlanner motion_;
    SafetyController safety_;
    std::array<AxisFeedback, protocol::AXIS_COUNT> feedback_{};
    std::array<uint64_t, protocol::AXIS_COUNT> feedback_age_us_{};
    std::array<bool, protocol::AXIS_COUNT> pending_move_{};
    std::array<uint64_t, protocol::AXIS_COUNT> move_age_us_{};
    bool reference_motion_enabled_{false};
    uint16_t monitor_faults_{0};
    bool heartbeat_seen_{false};
    uint64_t heartbeat_age_us_{spec::heartbeat_timeout_us};
    protocol::PacketParser parser_;
    protocol::SequenceWindow command_sequences_;
    uint64_t frame_age_us_{0};
    protocol::Packet response_{};
    bool response_pending_{false};
};

} // namespace starz::firmware
