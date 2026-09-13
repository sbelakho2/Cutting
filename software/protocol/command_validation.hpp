#pragma once
#include "starz_protocol.hpp"
#include "../../shared/machine_spec.hpp"

namespace starz::protocol {

inline constexpr uint8_t AXIS_COUNT = 4;
inline constexpr uint64_t INPUT_TIMEOUT_US = 50000;
inline constexpr uint64_t FRAME_TIMEOUT_US = 100000;
static_assert(spec::heartbeat_timeout_us == 750000, "Do not relax the heartbeat requirement");

constexpr int32_t axis_max_speed(uint8_t axis) {
    return axis < 2 ? spec::max_feed_um_s : spec::blade_speed_um_s;
}
constexpr int32_t axis_acceleration(uint8_t axis) {
    return axis < 2 ? spec::feed_accel_um_s2 : spec::blade_accel_um_s2;
}
constexpr int32_t axis_min_position(uint8_t axis) {
    return axis < 2 ? -spec::max_length_um : 0;
}
constexpr int32_t axis_max_position(uint8_t axis) {
    return axis < 2 ? spec::max_length_um : spec::blade_max_position_um;
}
constexpr bool position_in_range(uint8_t axis, int64_t position) {
    return axis < AXIS_COUNT && position >= axis_min_position(axis) && position <= axis_max_position(axis);
}
constexpr CommandResult validate_job(const JobStartPayload& job) {
    const auto valid_strip = [](int32_t strip) {
        return strip == 0 || (strip >= spec::min_strip_um && strip <= spec::max_strip_um);
    };
    if (job.wire_length_um < spec::min_length_um || job.wire_length_um > spec::max_length_um ||
        !valid_strip(job.lead_strip_um) || !valid_strip(job.tail_strip_um) ||
        job.blade_strip_depth_um <= 0 || job.blade_strip_depth_um >= spec::blade_cut_position_um ||
        job.target_quantity == 0 || job.target_quantity > spec::max_quantity ||
        static_cast<int64_t>(job.lead_strip_um) + job.tail_strip_um >= job.wire_length_um) {
        return CommandResult::InvalidArgument;
    }
    return CommandResult::Accepted;
}
constexpr CommandResult validate_jog(const JogAxisPayload& jog) {
    if (jog.axis_id >= AXIS_COUNT || (jog.direction != -1 && jog.direction != 1) ||
        jog.speed_um_s <= 0 || jog.speed_um_s > axis_max_speed(jog.axis_id) ||
        jog.distance_um <= 0 || jog.distance_um > axis_max_position(jog.axis_id)) {
        return CommandResult::InvalidArgument;
    }
    return CommandResult::Accepted;
}

// Saturates at a deadline without wrapping, including arbitrarily large tick gaps.
constexpr uint64_t age_by(uint64_t age, uint64_t delta, uint64_t limit) {
    return age >= limit || delta >= limit - age ? limit : age + delta;
}

} // namespace starz::protocol