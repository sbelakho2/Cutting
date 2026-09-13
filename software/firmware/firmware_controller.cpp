#include "firmware_controller.hpp"
#include <algorithm>

namespace starz::firmware {
namespace {
using protocol::CommandResult;
using protocol::Fault;
using protocol::MachineState;
using protocol::MsgId;

int expected_length(MsgId id) {
    switch (id) {
    case MsgId::CMD_PING:
    case MsgId::CMD_JOB_PAUSE:
    case MsgId::CMD_JOB_RESUME:
    case MsgId::CMD_JOB_ABORT:
    case MsgId::CMD_RECOVER:
    case MsgId::CMD_HOME_ALL:
    case MsgId::CMD_SOLENOID_DOOR: return 0;
    case MsgId::CMD_JOB_START: return protocol::JOB_PAYLOAD_SIZE;
    case MsgId::CMD_JOG_AXIS: return protocol::JOG_PAYLOAD_SIZE;
    case MsgId::CMD_SOLENOID_AIR: return 2;
    case MsgId::CMD_SET_CALIB: return 4;
    default: return -1;
    }
}

int64_t position_error(int32_t commanded, int32_t measured) {
    const int64_t difference = static_cast<int64_t>(commanded) - measured;
    return difference < 0 ? -difference : difference;
}
} // namespace

FirmwareController::FirmwareController(ReferenceMode mode) : mode_(mode) { init(); }

void FirmwareController::init() {
    // Repeated init is fail-closed, not a fault-reset escape hatch.
    stop_reference_motion();
    motion_.init();
    safety_.init();
    feedback_ = {};
    feedback_age_us_.fill(protocol::INPUT_TIMEOUT_US);
    heartbeat_seen_ = false;
    heartbeat_age_us_ = spec::heartbeat_timeout_us;
    latch(Fault::HeartbeatLost);
    latch(Fault::FeedbackUnknown);
    parser_.reset();
    frame_age_us_ = 0;
    response_pending_ = false;
    last_result_ = CommandResult::UnsafeState;
    // Do not reset the live sequence window or latched faults here.
    enforce_faults();
}

bool FirmwareController::heartbeat_fresh() const {
    return heartbeat_seen_ && heartbeat_age_us_ < spec::heartbeat_timeout_us;
}

bool FirmwareController::feedback_ready() const {
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        if (!feedback_[axis].valid || !feedback_[axis].measured_home_known ||
            feedback_age_us_[axis] >= protocol::INPUT_TIMEOUT_US ||
            !protocol::position_in_range(axis, feedback_[axis].position_um)) return false;
    }
    return true;
}

void FirmwareController::stop_reference_motion() {
    motion_.abort_all();
    reference_motion_enabled_ = false;
    pending_move_.fill(false);
    move_age_us_.fill(0);
}

void FirmwareController::enforce_faults() {
    if (faults() == 0) return;
    stop_reference_motion();
    motion_.invalidate_references();
    current_state_ = (faults() & protocol::fault_bit(Fault::Estop)) != 0 ? MachineState::ESTOP : MachineState::FAULT;
}

void FirmwareController::inject_inputs_for_test(const SafetyInputs& inputs) {
    safety_.update_inputs(inputs);
    enforce_faults(); // A trip is acted on in this call, even for a steady low input.
}

void FirmwareController::inject_feedback_for_test(const std::array<AxisFeedback, protocol::AXIS_COUNT>& feedback) {
    feedback_ = feedback; // The controller never creates encoder counts from commands.
    feedback_age_us_.fill(0);
    monitor_feedback();
    enforce_faults();
}

void FirmwareController::monitor_feedback() {
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        const auto& measured = feedback_[axis];
        if (!measured.valid || !measured.measured_home_known ||
            !protocol::position_in_range(axis, measured.position_um)) {
            latch(Fault::FeedbackUnknown);
            continue;
        }
        if (feedback_age_us_[axis] >= protocol::INPUT_TIMEOUT_US) {
            latch(Fault::FeedbackStale);
            continue;
        }
        if (motion_.home_known(axis)) {
            // Fixture-only following-error bounds, NOT qualified machine limits.
            const int64_t tolerance_um = axis < 2 ? 1000 : 100;
            const auto error = position_error(motion_.get_axis_pos_um(axis), measured.position_um);
            if (error > tolerance_um) latch(Fault::FollowingError);
            if (pending_move_[axis] && !motion_.axis_active(axis) && error == 0) pending_move_[axis] = false;
        }
    }
}

CommandResult FirmwareController::trigger_recovery() {
    // This acknowledgement does not create a job, start motion, or resume a move.
    if (!safety_.healthy_observation() || !heartbeat_fresh() || !feedback_ready() ||
        !motion_.is_idle() || reference_motion_enabled_ || safety_.unlock_requested()) {
        return remember(CommandResult::UnsafeState);
    }
    if (!safety_.deliberate_recovery()) return remember(CommandResult::UnsafeState);
    motion_.invalidate_references();
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        const auto result = motion_.establish_reference_for_test(axis, feedback_[axis].position_um,
                                                               feedback_[axis].measured_home_known);
        if (result != CommandResult::Accepted) {
            latch(Fault::FeedbackUnknown);
            enforce_faults();
            return remember(result);
        }
    }
    monitor_faults_ = 0;
    current_state_ = MachineState::IDLE;
    return remember(CommandResult::Accepted);
}

CommandResult FirmwareController::trigger_job_start(int32_t length_um, int32_t lead_um, int32_t tail_um,
                                                     int32_t depth_um, uint32_t quantity) {
    const auto validation = protocol::validate_job({length_um, lead_um, tail_um, depth_um, quantity});
    if (validation != CommandResult::Accepted) return remember(validation);
    // Deliberately no process state machine, timer-based completion or quality count.
    return remember(CommandResult::NotCommissioned);
}

CommandResult FirmwareController::trigger_jog(uint8_t axis_id, int8_t direction, int32_t speed_um_s,
                                              int32_t distance_um) {
    const auto validation = protocol::validate_jog({axis_id, direction, speed_um_s, distance_um});
    if (validation != CommandResult::Accepted) return remember(validation);
    if (mode_ != ReferenceMode::FaultInjection) return remember(CommandResult::NotCommissioned);
    if (!feedback_ready() || !motion_.home_known(axis_id)) return remember(CommandResult::HomeUnknown);
    if (faults() != 0 || !safety_.is_safe_to_move() || !heartbeat_fresh() ||
        (current_state_ != MachineState::IDLE && current_state_ != MachineState::RUNNING)) {
        return remember(CommandResult::UnsafeState);
    }
    if (pending_move_[axis_id]) return remember(CommandResult::Busy);
    const auto result = motion_.trigger_jog(axis_id, direction, speed_um_s, distance_um);
    if (result == CommandResult::Accepted) {
        pending_move_[axis_id] = true;
        move_age_us_[axis_id] = 0;
        reference_motion_enabled_ = true;
        current_state_ = MachineState::RUNNING;
    }
    return remember(result);
}

CommandResult FirmwareController::trigger_home() {
    return remember(feedback_ready() ? CommandResult::NotCommissioned : CommandResult::HomeUnknown);
}

CommandResult FirmwareController::trigger_abort() {
    stop_reference_motion();
    // A software abort is not evidence of measured standstill. Require recovery
    // with fresh feedback before any new reference motion.
    motion_.invalidate_references();
    latch(Fault::FeedbackUnknown);
    enforce_faults();
    return remember(CommandResult::Accepted);
}

CommandResult FirmwareController::trigger_pause() {
    // A resumable controlled stop is not modeled; conservatively abort the move.
    trigger_abort();
    return remember(CommandResult::NotCommissioned);
}

CommandResult FirmwareController::trigger_resume() {
    return remember(CommandResult::NotCommissioned);
}

CommandResult FirmwareController::request_door_unlock() {
    if (reference_motion_enabled_ || !motion_.is_idle()) return remember(CommandResult::UnsafeState);
    if (!safety_.request_door_unlock()) return remember(CommandResult::PermissionDenied);
    // Any unlock request invalidates the motion reference and requires recovery.
    latch(Fault::FeedbackUnknown);
    enforce_faults();
    return remember(CommandResult::Accepted);
}

void FirmwareController::update_tick(uint32_t delta_us) {
    safety_.elapse(delta_us);
    heartbeat_age_us_ = protocol::age_by(heartbeat_age_us_, delta_us, spec::heartbeat_timeout_us);
    if (!heartbeat_fresh()) latch(Fault::HeartbeatLost); // Trips at exactly 750000 us.
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        feedback_age_us_[axis] = protocol::age_by(feedback_age_us_[axis], delta_us, protocol::INPUT_TIMEOUT_US);
        if (pending_move_[axis]) {
            move_age_us_[axis] = protocol::age_by(move_age_us_[axis], delta_us, spec::move_timeout_us);
            if (move_age_us_[axis] >= spec::move_timeout_us) latch(Fault::MoveTimeout);
        }
    }
    if (parser_.partial()) {
        frame_age_us_ = protocol::age_by(frame_age_us_, delta_us, protocol::FRAME_TIMEOUT_US);
        if (frame_age_us_ >= protocol::FRAME_TIMEOUT_US) {
            parser_.reset();
            last_result_ = CommandResult::MalformedFrame;
        }
    }
    monitor_feedback();
    enforce_faults();
    if (!reference_motion_enabled_ || faults() != 0) return;
    motion_.update_tick(delta_us);
    monitor_feedback();
    enforce_faults();
    if (faults() != 0) return;
    if (motion_.is_idle() && std::none_of(pending_move_.begin(), pending_move_.end(), [](bool pending) { return pending; })) {
        reference_motion_enabled_ = false;
        current_state_ = MachineState::IDLE;
    }
}

void FirmwareController::process_incoming_byte(uint8_t byte) {
    const bool partial_before = parser_.partial();
    protocol::Packet packet;
    const auto result = parser_.consume(byte, packet);
    if (!partial_before && parser_.partial()) frame_age_us_ = 0;
    if (result == protocol::ParseResult::Complete) handle_packet(packet);
    else if (result == protocol::ParseResult::InvalidLength) last_result_ = CommandResult::InvalidLength;
    else if (result == protocol::ParseResult::BadCrc) last_result_ = CommandResult::MalformedFrame;
}

CommandResult FirmwareController::dispatch_command(const protocol::Packet& packet) {
    const auto id = static_cast<MsgId>(packet.msg_id);
    const int length = expected_length(id);
    if (length < 0) return CommandResult::Unsupported;
    if (packet.len != length) return CommandResult::InvalidLength;
    // An exact, valid ABORT is always honored, even if duplicated or out of order.
    if (id == MsgId::CMD_JOB_ABORT) return trigger_abort();
    // Do field validation before a rejected payload can consume a sequence slot.
    protocol::JobStartPayload job{};
    protocol::JogAxisPayload jog{};
    if (id == MsgId::CMD_JOB_START) {
        if (!protocol::decode_job(packet.payload.data(), packet.len, job)) return CommandResult::InvalidLength;
        const auto validation = protocol::validate_job(job);
        if (validation != CommandResult::Accepted) return validation;
    }
    if (id == MsgId::CMD_JOG_AXIS) {
        if (!protocol::decode_jog(packet.payload.data(), packet.len, jog)) return CommandResult::InvalidLength;
        const auto validation = protocol::validate_jog(jog);
        if (validation != CommandResult::Accepted) return validation;
    }
    if (id == MsgId::CMD_SET_CALIB && protocol::get_i32_le(packet.payload.data()) <= 0) return CommandResult::InvalidArgument;
    if (id == MsgId::CMD_SOLENOID_AIR && (protocol::get_u16_le(packet.payload.data()) == 0 ||
                                        protocol::get_u16_le(packet.payload.data()) > 1000)) return CommandResult::InvalidArgument;
    if (!command_sequences_.accept(packet.seq)) return CommandResult::Replay;
    switch (id) {
    case MsgId::CMD_PING:
        heartbeat_seen_ = true;
        heartbeat_age_us_ = 0; // Only a new, exactly formed ping refreshes the deadline.
        return CommandResult::Accepted;
    case MsgId::CMD_JOB_START:
        return trigger_job_start(job.wire_length_um, job.lead_strip_um, job.tail_strip_um,
                                 job.blade_strip_depth_um, job.target_quantity);
    case MsgId::CMD_JOG_AXIS:
        return trigger_jog(jog.axis_id, jog.direction, jog.speed_um_s, jog.distance_um);
    case MsgId::CMD_JOB_PAUSE: return trigger_pause();
    case MsgId::CMD_JOB_RESUME: return trigger_resume();
    case MsgId::CMD_RECOVER: return trigger_recovery();
    case MsgId::CMD_HOME_ALL: return trigger_home();
    case MsgId::CMD_SOLENOID_DOOR: return request_door_unlock();
    case MsgId::CMD_SET_CALIB:
    case MsgId::CMD_SOLENOID_AIR: return CommandResult::NotCommissioned;
    default: return CommandResult::Unsupported;
    }
}

void FirmwareController::handle_packet(const protocol::Packet& packet) {
    last_result_ = dispatch_command(packet);
    response_ = {};
    response_.seq = packet.seq;
    if (packet.msg_id == static_cast<uint8_t>(MsgId::CMD_PING) && last_result_ == CommandResult::Accepted) {
        response_.msg_id = static_cast<uint8_t>(MsgId::RESP_PONG);
    } else {
        response_.msg_id = static_cast<uint8_t>(last_result_ == CommandResult::Accepted ? MsgId::RESP_ACK : MsgId::RESP_NACK);
        response_.len = 1;
        response_.payload[0] = static_cast<uint8_t>(last_result_);
    }
    response_pending_ = true; // Bounded one-response mailbox; caller must drain it.
}

bool FirmwareController::take_response(protocol::Packet& packet) {
    if (!response_pending_) return false;
    packet = response_;
    response_pending_ = false;
    return true;
}

protocol::TelemetryPayload FirmwareController::get_telemetry() const {
    protocol::TelemetryPayload telemetry{};
    telemetry.state = static_cast<uint8_t>(current_state_);
    telemetry.safety_flags = safety_.get_flags_byte();
    telemetry.feed_command_um = motion_.get_feed_pos_um();
    telemetry.encoder_meas_um = feedback_[0].valid && feedback_age_us_[0] < protocol::INPUT_TIMEOUT_US &&
                                protocol::position_in_range(0, feedback_[0].position_um)
                                    ? feedback_[0].position_um : protocol::UNKNOWN_POSITION_UM;
    telemetry.blade_front_um = motion_.get_blade_front_pos_um();
    telemetry.blade_rear_um = motion_.get_blade_rear_pos_um();
    telemetry.error_code = faults();
    // target_pcs, completed_pcs and cycle_time_ms stay zero: no process is modeled.
    return telemetry;
}

} // namespace starz::firmware
