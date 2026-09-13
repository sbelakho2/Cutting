#include "motion_planner.hpp"
#include <algorithm>
#include <cmath>

namespace starz::firmware {

MotionPlanner::MotionPlanner() = default;

void MotionPlanner::init() { axes_ = {}; }

protocol::CommandResult MotionPlanner::configure_job(int32_t length_um, int32_t lead_strip_um,
                                                     int32_t tail_strip_um, int32_t cut_depth_um) {
    const auto validation = protocol::validate_job({length_um, lead_strip_um, tail_strip_um, cut_depth_um, 1});
    return validation == protocol::CommandResult::Accepted ? protocol::CommandResult::NotCommissioned : validation;
}

protocol::CommandResult MotionPlanner::establish_reference_for_test(uint8_t axis_id, int32_t measured_um,
                                                                   bool measured_home_known) {
    if (axis_id >= axes_.size()) return protocol::CommandResult::InvalidArgument;
    if (!is_idle()) return protocol::CommandResult::Busy;
    if (!measured_home_known) return protocol::CommandResult::HomeUnknown;
    if (!protocol::position_in_range(axis_id, measured_um)) return protocol::CommandResult::OutOfRange;
    auto& axis = axes_[axis_id];
    axis = {};
    axis.start_pos_um = measured_um;
    axis.target_pos_um = measured_um;
    axis.current_pos_um = measured_um;
    axis.home_known = true;
    return protocol::CommandResult::Accepted;
}

protocol::CommandResult MotionPlanner::trigger_jog(uint8_t axis_id, int8_t dir, int32_t speed_um_s,
                                                   int32_t distance_um) {
    const auto validation = protocol::validate_jog({axis_id, dir, speed_um_s, distance_um});
    if (validation != protocol::CommandResult::Accepted) return validation;
    auto& axis = axes_[axis_id];
    if (!axis.home_known) return protocol::CommandResult::HomeUnknown;
    if (axis.active) return protocol::CommandResult::Busy;
    const int64_t target = static_cast<int64_t>(axis.current_pos_um) + static_cast<int64_t>(dir) * distance_um;
    if (!protocol::position_in_range(axis_id, target)) return protocol::CommandResult::OutOfRange;

    ProfileSegment next{};
    next.start_pos_um = axis.current_pos_um;
    next.current_pos_um = axis.current_pos_um;
    next.target_pos_um = static_cast<int32_t>(target);
    next.direction = dir;
    next.max_speed_um_s = speed_um_s;
    next.accel_um_s2 = protocol::axis_acceleration(axis_id);
    next.distance_um = static_cast<double>(distance_um);
    const double acceleration = static_cast<double>(next.accel_um_s2);
    // Analytic triangular/trapezoidal trajectory from total integer elapsed time.
    // No Euler integration, tick-dependent rounding accumulation or creep floor.
    next.peak_speed_um_s = std::min(static_cast<double>(speed_um_s), std::sqrt(next.distance_um * acceleration));
    next.acceleration_time_s = next.peak_speed_um_s / acceleration;
    const double ramp_distance = next.peak_speed_um_s * next.peak_speed_um_s / acceleration;
    next.cruise_time_s = std::max(0.0, (next.distance_um - ramp_distance) / next.peak_speed_um_s);
    next.total_time_s = 2.0 * next.acceleration_time_s + next.cruise_time_s;
    const double duration = std::ceil(next.total_time_s * 1000000.0);
    if (!std::isfinite(duration) || duration > static_cast<double>(spec::move_timeout_us)) {
        return protocol::CommandResult::OutOfRange;
    }
    next.duration_us = static_cast<uint64_t>(duration);
    next.home_known = true;
    next.active = true;
    axis = next;
    return protocol::CommandResult::Accepted;
}

void MotionPlanner::step_axis(ProfileSegment& axis, uint32_t delta_us) {
    if (!axis.active || delta_us == 0) return;
    axis.elapsed_us = protocol::age_by(axis.elapsed_us, delta_us, axis.duration_us);
    if (axis.elapsed_us >= axis.duration_us) {
        axis.current_pos_um = axis.target_pos_um;
        axis.current_speed_um_s = 0;
        axis.active = false;
        axis.complete = true; // Profile only; not measured arrival or part acceptance.
        return;
    }

    const double time = static_cast<double>(axis.elapsed_us) / 1000000.0;
    const double acceleration = static_cast<double>(axis.accel_um_s2);
    const double ramp_time = axis.acceleration_time_s;
    double distance = 0.0;
    double speed = 0.0;
    if (time < ramp_time) {
        distance = 0.5 * acceleration * time * time;
        speed = acceleration * time;
    } else if (time < ramp_time + axis.cruise_time_s) {
        distance = 0.5 * acceleration * ramp_time * ramp_time + axis.peak_speed_um_s * (time - ramp_time);
        speed = axis.peak_speed_um_s;
    } else {
        const double remaining = std::max(0.0, axis.total_time_s - time);
        distance = axis.distance_um - 0.5 * acceleration * remaining * remaining;
        speed = acceleration * remaining;
    }
    const auto whole_um = static_cast<int64_t>(std::floor(std::clamp(distance, 0.0, axis.distance_um)));
    const int64_t position = static_cast<int64_t>(axis.start_pos_um) + static_cast<int64_t>(axis.direction) * whole_um;
    axis.current_pos_um = static_cast<int32_t>(position); // Both endpoints were range checked.
    axis.current_speed_um_s = static_cast<int32_t>(std::clamp(speed, 0.0, static_cast<double>(axis.max_speed_um_s))) * axis.direction;
}

bool MotionPlanner::update_tick(uint32_t delta_us) {
    for (auto& axis : axes_) step_axis(axis, delta_us);
    return !is_idle();
}

protocol::CommandResult MotionPlanner::trigger_home() {
    return protocol::CommandResult::NotCommissioned;
}

void MotionPlanner::abort_all() {
    for (auto& axis : axes_) {
        axis.active = false;
        axis.complete = false;
        axis.current_speed_um_s = 0;
    }
}

void MotionPlanner::invalidate_references() {
    abort_all();
    for (auto& axis : axes_) axis.home_known = false;
}

bool MotionPlanner::is_idle() const {
    return std::none_of(axes_.begin(), axes_.end(), [](const auto& axis) { return axis.active; });
}

int32_t MotionPlanner::get_axis_pos_um(uint8_t axis_id) const {
    return home_known(axis_id) ? axes_[axis_id].current_pos_um : protocol::UNKNOWN_POSITION_UM;
}

int32_t MotionPlanner::get_axis_speed_um_s(uint8_t axis_id) const {
    return axis_id < axes_.size() ? axes_[axis_id].current_speed_um_s : 0;
}

bool MotionPlanner::axis_active(uint8_t axis_id) const {
    return axis_id < axes_.size() && axes_[axis_id].active;
}

bool MotionPlanner::home_known(uint8_t axis_id) const {
    return axis_id < axes_.size() && axes_[axis_id].home_known;
}

} // namespace starz::firmware
