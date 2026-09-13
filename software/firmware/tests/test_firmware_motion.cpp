#include "reference_fixture.hpp"
#include <array>
#include <iostream>
#include <limits>

namespace {
using namespace starz;
using namespace starz::tests;

void test_always_active_checks() {
    bool caught = false;
    try { CHECK(false); } catch (const std::runtime_error&) { caught = true; }
    CHECK(caught);
}

void test_unknown_startup_and_recovery() {
    firmware::FirmwareController controller;
    CHECK(controller.get_state() == MachineState::FAULT);
    CHECK(controller.get_telemetry().safety_flags == protocol::REFERENCE_ONLY_FLAG);
    CHECK(controller.get_telemetry().encoder_meas_um == protocol::UNKNOWN_POSITION_UM);
    CHECK(controller.get_telemetry().feed_command_um == protocol::UNKNOWN_POSITION_UM);
    CHECK(controller.trigger_recovery() == CommandResult::UnsafeState);
    CHECK(controller.trigger_home() == CommandResult::HomeUnknown);
    CHECK(controller.request_door_unlock() == CommandResult::PermissionDenied);
    CHECK(controller.trigger_job_start(500000, 10000, 10000, 3000, 2) == CommandResult::NotCommissioned);
    CHECK(controller.trigger_jog(0, 1, 10000, 1000) == CommandResult::NotCommissioned);
    CHECK(controller.trigger_resume() == CommandResult::NotCommissioned);
    controller.update_tick(std::numeric_limits<uint32_t>::max());
    CHECK(controller.get_state() == MachineState::FAULT);
    CHECK(controller.get_telemetry().completed_pcs == 0);

    controller.inject_inputs_for_test(healthy_inputs());
    auto feedback = measured_home_fixture();
    feedback[2].measured_home_known = false;
    controller.inject_feedback_for_test(feedback);
    CHECK(deliver(controller, packet(MsgId::CMD_PING, 0)) == CommandResult::Accepted);
    CHECK(controller.trigger_recovery() == CommandResult::UnsafeState);
    feedback[2].measured_home_known = true;
    controller.inject_feedback_for_test(feedback);
    CHECK(controller.get_state() == MachineState::FAULT); // Samples/ping alone never reset.
    CHECK(controller.trigger_recovery() == CommandResult::Accepted);
    CHECK(controller.get_state() == MachineState::IDLE);
    CHECK(controller.trigger_jog(0, 1, 10000, 1000) == CommandResult::NotCommissioned);
    CHECK(controller.trigger_home() == CommandResult::NotCommissioned);
    controller.init();
    CHECK(controller.get_state() == MachineState::FAULT);
    CHECK(!controller.hardware_outputs_enabled());
}

void test_safety_observations_and_permission() {
    firmware::SafetyController safety;
    CHECK(!safety.is_safe_to_move());
    CHECK(!safety.request_door_unlock());
    auto inputs = healthy_inputs();
    safety.update_inputs(inputs);
    CHECK(!safety.is_safe_to_move());
    CHECK(safety.deliberate_recovery());
    CHECK(safety.is_safe_to_move());
    CHECK(!safety.request_door_unlock());
    CHECK(safety.measured_inputs().guard_locked);
    inputs.guard_locked = false;
    safety.update_inputs(inputs);
    safety.lock_door();
    CHECK(!safety.measured_inputs().guard_locked); // Command cannot spoof a lock.
    CHECK(!safety.deliberate_recovery());
    CHECK(safety.evaluate_state(MachineState::RUNNING) == MachineState::FAULT);
    inputs.guard_locked = true;
    safety.update_inputs(inputs);
    CHECK(!safety.is_safe_to_move());
    CHECK(safety.deliberate_recovery());
    inputs.independent_unlock_permission = true;
    safety.update_inputs(inputs);
    CHECK(safety.request_door_unlock());
    CHECK(safety.unlock_requested());
    CHECK(safety.measured_inputs().guard_locked);
    CHECK(!safety.is_safe_to_move());
    CHECK(!safety.deliberate_recovery());
    safety.elapse(protocol::INPUT_TIMEOUT_US);
    CHECK(!safety.unlock_requested());
    CHECK(!safety.request_door_unlock());

    // Trip even when the very first sample is low and on every repeated low level.
    firmware::SafetyController estop;
    estop.update_inputs(false, true, true, true, false);
    CHECK(estop.evaluate_state(MachineState::IDLE) == MachineState::ESTOP);
    estop.update_inputs(false, true, true, true, false);
    CHECK(!estop.deliberate_recovery());
    estop.init();
    CHECK(estop.evaluate_state(MachineState::IDLE) == MachineState::ESTOP);
    estop.update_inputs(true, true, true, true, false);
    CHECK(estop.evaluate_state(MachineState::IDLE) == MachineState::ESTOP);
    CHECK(estop.deliberate_recovery());
    CHECK(estop.is_safe_to_move());
}

void test_immediate_sticky_fault_injection() {
    for (const auto fault : {Fault::Estop, Fault::GuardOpen, Fault::DriverAlarm, Fault::WireMissing,
                             Fault::SlugJam, Fault::InputsUnknown}) {
        ReferenceFixture fixture;
        auto& controller = fixture.controller;
        CHECK(controller.trigger_jog(0, 1, 10000, 10000) == CommandResult::Accepted);
        fixture.advance(1000, true);
        CHECK(controller.reference_motion_active());
        auto broken = fixture.inputs;
        switch (fault) {
        case Fault::Estop: broken.safety_relay_ok = false; break;
        case Fault::GuardOpen: broken.guard_locked = false; break;
        case Fault::DriverAlarm: broken.driver_alarm = true; break;
        case Fault::WireMissing: broken.wire_present = false; break;
        case Fault::SlugJam: broken.slug_clear = false; break;
        default: broken.valid = false; break;
        }
        controller.inject_inputs_for_test(broken);
        // No tick, rising edge or debounce is needed to revoke reference motion.
        CHECK(!controller.reference_motion_active());
        CHECK(has_fault(controller, fault));
        CHECK(controller.get_state() == (fault == Fault::Estop ? MachineState::ESTOP : MachineState::FAULT));
        CHECK(controller.trigger_recovery() == CommandResult::UnsafeState);
        controller.inject_inputs_for_test(broken);
        CHECK(!controller.reference_motion_active());
        fixture.sample();
        fixture.ping();
        CHECK(has_fault(controller, fault));
        CHECK(controller.trigger_resume() == CommandResult::NotCommissioned);
        CHECK(controller.trigger_job_start(500000, 10000, 10000, 3000, 2) == CommandResult::NotCommissioned);
        CHECK(has_fault(controller, fault));
        CHECK(controller.trigger_recovery() == CommandResult::Accepted);
        CHECK(controller.get_state() == MachineState::IDLE);
        fixture.advance(1000);
        CHECK(!controller.reference_motion_active()); // Recovery is never restart.
        CHECK(controller.trigger_jog(0, 1, 10000, 100) == CommandResult::Accepted);
        CHECK(controller.trigger_abort() == CommandResult::Accepted);
        CHECK(!controller.reference_motion_active());
        CHECK(controller.get_state() == MachineState::FAULT);
    }
}

void test_deadlines_and_missing_samples() {
    ReferenceFixture heartbeat;
    CHECK(heartbeat.controller.trigger_jog(0, 1, 10000, 100000) == CommandResult::Accepted);
    for (int i = 0; i < 749; ++i) heartbeat.advance(1000, true, false);
    heartbeat.advance(999, true, false);
    CHECK(!has_fault(heartbeat.controller, Fault::HeartbeatLost));
    CHECK(heartbeat.controller.reference_motion_active());
    heartbeat.advance(1, true, false);
    CHECK(has_fault(heartbeat.controller, Fault::HeartbeatLost));
    CHECK(!heartbeat.controller.reference_motion_active());
    heartbeat.ping();
    CHECK(has_fault(heartbeat.controller, Fault::HeartbeatLost));
    heartbeat.sample();
    CHECK(heartbeat.controller.trigger_recovery() == CommandResult::Accepted);
    CHECK(!heartbeat.controller.reference_motion_active());

    ReferenceFixture stale;
    stale.controller.update_tick(static_cast<uint32_t>(protocol::INPUT_TIMEOUT_US - 1));
    CHECK(stale.controller.faults() == 0);
    stale.controller.update_tick(1);
    CHECK(has_fault(stale.controller, Fault::InputsStale));
    CHECK(has_fault(stale.controller, Fault::FeedbackStale));
    CHECK((stale.controller.get_telemetry().safety_flags & protocol::INPUTS_FRESH_FLAG) == 0);
    CHECK(stale.controller.get_telemetry().encoder_meas_um == protocol::UNKNOWN_POSITION_UM);
    CHECK(stale.controller.trigger_recovery() == CommandResult::UnsafeState);

    ReferenceFixture missing_feedback;
    missing_feedback.controller.update_tick(49000);
    missing_feedback.controller.inject_inputs_for_test(missing_feedback.inputs);
    missing_feedback.controller.update_tick(1000);
    CHECK(!has_fault(missing_feedback.controller, Fault::InputsStale));
    CHECK(has_fault(missing_feedback.controller, Fault::FeedbackStale));

    ReferenceFixture missing_inputs;
    missing_inputs.controller.update_tick(49000);
    missing_inputs.controller.inject_feedback_for_test(missing_inputs.feedback);
    missing_inputs.controller.update_tick(1000);
    CHECK(has_fault(missing_inputs.controller, Fault::InputsStale));
    CHECK(!has_fault(missing_inputs.controller, Fault::FeedbackStale));

    ReferenceFixture huge_gap;
    huge_gap.controller.update_tick(std::numeric_limits<uint32_t>::max());
    huge_gap.controller.update_tick(std::numeric_limits<uint32_t>::max());
    CHECK(has_fault(huge_gap.controller, Fault::HeartbeatLost));
    CHECK(!huge_gap.controller.reference_motion_active());
}

void test_feedback_faults_and_no_invented_parts() {
    ReferenceFixture stuck;
    CHECK(stuck.controller.trigger_jog(0, 1, 25000, 10000) == CommandResult::Accepted);
    for (int i = 0; i < 100 && stuck.controller.reference_motion_active(); ++i) stuck.advance(1000);
    CHECK(has_fault(stuck.controller, Fault::FollowingError));
    CHECK(stuck.controller.get_telemetry().encoder_meas_um == 0);
    CHECK(stuck.controller.get_telemetry().completed_pcs == 0);

    ReferenceFixture tiny_stuck;
    CHECK(tiny_stuck.controller.trigger_jog(0, 1, 1000, 1) == CommandResult::Accepted);
    for (int i = 0; i < 29999; ++i) tiny_stuck.advance(1000);
    CHECK(tiny_stuck.controller.reference_motion_active());
    CHECK(!has_fault(tiny_stuck.controller, Fault::MoveTimeout));
    tiny_stuck.advance(1000);
    CHECK(has_fault(tiny_stuck.controller, Fault::MoveTimeout));
    CHECK(!tiny_stuck.controller.reference_motion_active());

    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        ReferenceFixture failed;
        CHECK(failed.controller.trigger_jog(axis, 1, 10000, 1000) == CommandResult::Accepted);
        failed.feedback[axis].valid = false;
        failed.controller.inject_feedback_for_test(failed.feedback);
        CHECK(has_fault(failed.controller, Fault::FeedbackUnknown));
        CHECK(!failed.controller.reference_motion_active());
    }

    ReferenceFixture home_lost;
    home_lost.feedback[3].measured_home_known = false;
    home_lost.controller.inject_feedback_for_test(home_lost.feedback);
    CHECK(home_lost.controller.trigger_recovery() == CommandResult::UnsafeState);
    CHECK(home_lost.controller.trigger_home() == CommandResult::HomeUnknown);
    home_lost.feedback[3] = {true, true, std::numeric_limits<int32_t>::max()};
    home_lost.controller.inject_feedback_for_test(home_lost.feedback);
    CHECK(home_lost.controller.trigger_recovery() == CommandResult::UnsafeState);

    ReferenceFixture ideal;
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        CHECK(ideal.controller.trigger_jog(axis, 1, 10000, 1000) == CommandResult::Accepted);
    }
    CHECK(ideal.controller.trigger_job_start(500000, 10000, 10000, 3000, 2) == CommandResult::NotCommissioned);
    for (int i = 0; i < 2000 && ideal.controller.reference_motion_active(); ++i) ideal.advance(1000, true);
    CHECK(ideal.controller.faults() == 0);
    CHECK(ideal.controller.get_state() == MachineState::IDLE);
    CHECK(ideal.controller.get_telemetry().completed_pcs == 0);
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) CHECK(ideal.controller.reference_command_um(axis) == 1000);
}

void test_controller_input_validation() {
    ReferenceFixture fixture;
    auto& controller = fixture.controller;
    constexpr int32_t low = std::numeric_limits<int32_t>::min();
    constexpr int32_t high = std::numeric_limits<int32_t>::max();
    const protocol::JobStartPayload good{500000, 10000, 10000, 3000, 2};
    for (const auto value : {low, -1, 0, 199999, 3000001, high}) {
        CHECK(controller.trigger_job_start(value, good.lead_strip_um, good.tail_strip_um, good.blade_strip_depth_um, good.target_quantity) == CommandResult::InvalidArgument);
    }
    for (const auto value : {low, -1, 1, 2999, 25001, high}) {
        CHECK(controller.trigger_job_start(good.wire_length_um, value, good.tail_strip_um, good.blade_strip_depth_um, good.target_quantity) == CommandResult::InvalidArgument);
        CHECK(controller.trigger_job_start(good.wire_length_um, good.lead_strip_um, value, good.blade_strip_depth_um, good.target_quantity) == CommandResult::InvalidArgument);
    }
    for (const auto value : {low, -1, 0, spec::blade_cut_position_um, high}) {
        CHECK(controller.trigger_job_start(good.wire_length_um, good.lead_strip_um, good.tail_strip_um, value, good.target_quantity) == CommandResult::InvalidArgument);
    }
    for (const uint32_t quantity : {0U, spec::max_quantity + 1U, std::numeric_limits<uint32_t>::max()}) {
        CHECK(controller.trigger_job_start(good.wire_length_um, good.lead_strip_um, good.tail_strip_um, good.blade_strip_depth_um, quantity) == CommandResult::InvalidArgument);
    }
    CHECK(controller.trigger_job_start(200000, 0, 0, 1, 1) == CommandResult::NotCommissioned);
    CHECK(controller.trigger_job_start(3000000, 25000, 25000, 4099, 1000000) == CommandResult::NotCommissioned);

    for (const auto axis : {4U, 255U}) CHECK(controller.trigger_jog(static_cast<uint8_t>(axis), 1, 10000, 1000) == CommandResult::InvalidArgument);
    for (const int direction : {-128, -2, 0, 2, 127}) CHECK(controller.trigger_jog(0, static_cast<int8_t>(direction), 10000, 1000) == CommandResult::InvalidArgument);
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        for (const int32_t value : {low, -1, 0, high}) {
            CHECK(controller.trigger_jog(axis, 1, value, 1000) == CommandResult::InvalidArgument);
            CHECK(controller.trigger_jog(axis, 1, 10000, value) == CommandResult::InvalidArgument);
        }
        CHECK(controller.trigger_jog(axis, 1, protocol::axis_max_speed(axis) + 1, 1000) == CommandResult::InvalidArgument);
        CHECK(controller.trigger_jog(axis, 1, 10000, protocol::axis_max_position(axis) + 1) == CommandResult::InvalidArgument);
    }
    CHECK(controller.get_state() == MachineState::IDLE);
    CHECK(controller.trigger_jog(2, -1, 10000, 1000) == CommandResult::OutOfRange);
    CHECK(controller.trigger_jog(0, 1, 1, 1000) == CommandResult::OutOfRange); // >30 s.
    CHECK(!controller.reference_motion_active());
    CHECK(controller.trigger_jog(0, 1, 10000, 1000) == CommandResult::Accepted);
    CHECK(controller.trigger_jog(0, -1, 10000, 1000) == CommandResult::Busy);
    CHECK(controller.trigger_pause() == CommandResult::NotCommissioned);
    CHECK(!controller.reference_motion_active());
    CHECK(controller.trigger_resume() == CommandResult::NotCommissioned);
    CHECK(!controller.reference_motion_active());
}

firmware::MotionPlanner referenced_planner() {
    firmware::MotionPlanner planner;
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) {
        CHECK(planner.establish_reference_for_test(axis, 0, true) == CommandResult::Accepted);
    }
    return planner;
}

void test_analytic_four_axis_profiles() {
    firmware::MotionPlanner unknown;
    CHECK(unknown.trigger_home() == CommandResult::NotCommissioned);
    CHECK(unknown.get_feed_pos_um() == protocol::UNKNOWN_POSITION_UM);
    CHECK(unknown.trigger_jog(0, 1, 10000, 1000) == CommandResult::HomeUnknown);
    CHECK(unknown.establish_reference_for_test(0, 0, false) == CommandResult::HomeUnknown);
    CHECK(unknown.establish_reference_for_test(4, 0, true) == CommandResult::InvalidArgument);
    CHECK(unknown.establish_reference_for_test(0, std::numeric_limits<int32_t>::max(), true) == CommandResult::OutOfRange);

    auto axes = referenced_planner();
    CHECK(axes.trigger_jog(0, 1, 10000, 1000) == CommandResult::Accepted);
    CHECK(axes.trigger_jog(1, -1, 20000, 2000) == CommandResult::Accepted);
    CHECK(axes.trigger_jog(2, 1, 10000, 500) == CommandResult::Accepted);
    CHECK(axes.trigger_jog(3, 1, 10000, 750) == CommandResult::Accepted);
    CHECK(axes.establish_reference_for_test(0, 0, true) == CommandResult::Busy);
    CHECK(!axes.update_tick(1000000));
    CHECK(axes.get_feed_pos_um() == 1000);
    CHECK(axes.get_feed_right_pos_um() == -2000);
    CHECK(axes.get_blade_front_pos_um() == 500);
    CHECK(axes.get_blade_rear_pos_um() == 750);
    for (uint8_t axis = 0; axis < protocol::AXIS_COUNT; ++axis) CHECK(axes.get_axis_speed_um_s(axis) == 0);

    // Fixture expected values: a=1.5e6, v=300000, d=600000, ramps=0.2 s,
    // cruise=1.8 s, total=2.2 s. Check ramp, cruise, deceleration and exact end.
    auto trapezoid = referenced_planner();
    CHECK(trapezoid.trigger_jog(0, 1, 300000, 600000) == CommandResult::Accepted);
    CHECK(trapezoid.update_tick(100000));
    CHECK(trapezoid.get_feed_pos_um() == 7500);
    CHECK(trapezoid.get_axis_speed_um_s(0) == 150000);
    CHECK(trapezoid.update_tick(100000));
    CHECK(trapezoid.get_feed_pos_um() == 30000);
    CHECK(trapezoid.get_axis_speed_um_s(0) == 300000);
    CHECK(trapezoid.update_tick(1800000));
    CHECK(trapezoid.get_feed_pos_um() == 570000);
    CHECK(trapezoid.update_tick(100000));
    CHECK(trapezoid.get_feed_pos_um() >= 592499 && trapezoid.get_feed_pos_um() <= 592500);
    CHECK(trapezoid.get_axis_speed_um_s(0) >= 149999 && trapezoid.get_axis_speed_um_s(0) <= 150000);
    CHECK(!trapezoid.update_tick(100001));
    CHECK(trapezoid.get_feed_pos_um() == 600000);
    CHECK(trapezoid.get_axis_speed_um_s(0) == 0);

    auto triangle = referenced_planner();
    CHECK(triangle.trigger_jog(0, 1, 300000, 15000) == CommandResult::Accepted);
    CHECK(triangle.update_tick(100000));
    CHECK(triangle.get_feed_pos_um() == 7500);
    CHECK(triangle.get_axis_speed_um_s(0) == 150000);
    CHECK(!triangle.update_tick(100001));
    CHECK(triangle.get_feed_pos_um() == 15000);

    auto low_speed = referenced_planner();
    CHECK(low_speed.trigger_jog(0, 1, 1, 1) == CommandResult::Accepted);
    CHECK(low_speed.update_tick(0));
    CHECK(low_speed.get_feed_pos_um() == 0);
    CHECK(low_speed.get_axis_speed_um_s(0) == 0);
    CHECK(low_speed.update_tick(500000));
    CHECK(low_speed.get_feed_pos_um() == 0); // No fabricated creep displacement.
    CHECK(low_speed.get_axis_speed_um_s(0) <= 1);
    CHECK(!low_speed.update_tick(500002));
    CHECK(low_speed.get_feed_pos_um() == 1);

    auto large_tick = referenced_planner();
    auto small_ticks = referenced_planner();
    CHECK(large_tick.trigger_jog(0, -1, 200000, 123456) == CommandResult::Accepted);
    CHECK(small_ticks.trigger_jog(0, -1, 200000, 123456) == CommandResult::Accepted);
    CHECK(large_tick.update_tick(333333));
    for (int i = 0; i < 333; ++i) CHECK(small_ticks.update_tick(1000));
    CHECK(small_ticks.update_tick(333));
    CHECK(large_tick.get_feed_pos_um() == small_ticks.get_feed_pos_um());
    CHECK(large_tick.get_axis_speed_um_s(0) == small_ticks.get_axis_speed_um_s(0));
    CHECK(!large_tick.update_tick(std::numeric_limits<uint32_t>::max()));
    CHECK(large_tick.get_feed_pos_um() == -123456);
    CHECK(large_tick.configure_job(500000, 10000, 10000, 3000) == CommandResult::NotCommissioned);
    CHECK(large_tick.is_idle());
    large_tick.abort_all();
    large_tick.invalidate_references();
    CHECK(large_tick.get_feed_pos_um() == protocol::UNKNOWN_POSITION_UM);

    auto boundary = referenced_planner();
    CHECK(boundary.establish_reference_for_test(0, spec::max_length_um, true) == CommandResult::Accepted);
    CHECK(boundary.trigger_jog(0, 1, 10000, 1) == CommandResult::OutOfRange);
    CHECK(boundary.establish_reference_for_test(1, -spec::max_length_um, true) == CommandResult::Accepted);
    CHECK(boundary.trigger_jog(1, -1, 10000, 1) == CommandResult::OutOfRange);
}

void test_controller_unlock_and_reset_quarantine() {
    ReferenceFixture fixture;
    CHECK(fixture.controller.request_door_unlock() == CommandResult::PermissionDenied);
    fixture.inputs.independent_unlock_permission = true;
    fixture.sample();
    CHECK(fixture.controller.request_door_unlock() == CommandResult::Accepted);
    CHECK(fixture.controller.unlock_requested());
    CHECK(fixture.controller.measured_inputs().guard_locked);
    CHECK(fixture.controller.trigger_recovery() == CommandResult::UnsafeState);
    fixture.controller.request_door_lock();
    CHECK(fixture.controller.measured_inputs().guard_locked);
    CHECK(fixture.controller.trigger_recovery() == CommandResult::Accepted);
    CHECK(fixture.controller.trigger_jog(0, 1, 10000, 1000) == CommandResult::Accepted);
    CHECK(fixture.controller.request_door_unlock() == CommandResult::UnsafeState);
    fixture.inputs.safety_relay_ok = false;
    fixture.sample();
    fixture.controller.init();
    CHECK(has_fault(fixture.controller, Fault::Estop));
    CHECK(fixture.controller.get_state() == MachineState::ESTOP);
    CHECK(!fixture.controller.reference_motion_active());
}
} // namespace

int main() {
    try {
        test_always_active_checks();
        test_unknown_startup_and_recovery();
        test_safety_observations_and_permission();
        test_immediate_sticky_fault_injection();
        test_deadlines_and_missing_samples();
        test_feedback_faults_and_no_invented_parts();
        test_controller_input_validation();
        test_analytic_four_axis_profiles();
        test_controller_unlock_and_reset_quarantine();
        std::cout << "PASS: 9 non-hardware reference control test groups; checks active in Debug and Release.\n"
                     "No physical stopping time, safety certification or production quality was tested.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
