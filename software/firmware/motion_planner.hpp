#pragma once
#include <array>
#include <cstdint>
#include "reference_only.hpp"
#include "../protocol/command_validation.hpp"

namespace starz::firmware {

struct ProfileSegment {
    int32_t start_pos_um{0};
    int32_t target_pos_um{0};
    int32_t max_speed_um_s{0};
    int32_t accel_um_s2{0};
    int32_t current_pos_um{0};
    int32_t current_speed_um_s{0};
    bool active{false};
    bool complete{false};
    bool home_known{false};
    int8_t direction{1};
    uint64_t elapsed_us{0};
    uint64_t duration_us{0};
    double distance_um{0.0};
    double peak_speed_um_s{0.0};
    double acceleration_time_s{0.0};
    double cruise_time_s{0.0};
    double total_time_s{0.0};
};

class MotionPlanner {
public:
    MotionPlanner();

    void init();
    protocol::CommandResult configure_job(int32_t length_um, int32_t lead_strip_um, int32_t tail_strip_um, int32_t cut_depth_um);
    
    // Step forward by delta_t microseconds, returns true if axis still moving
    bool update_tick(uint32_t delta_us);

    // Axis targets and status
    int32_t get_axis_pos_um(uint8_t axis_id) const;
    int32_t get_axis_speed_um_s(uint8_t axis_id) const;
    bool axis_active(uint8_t axis_id) const;
    bool home_known(uint8_t axis_id) const;
    int32_t get_feed_pos_um() const { return get_axis_pos_um(0); }
    int32_t get_feed_right_pos_um() const { return get_axis_pos_um(1); }
    int32_t get_blade_front_pos_um() const { return get_axis_pos_um(2); }
    int32_t get_blade_rear_pos_um() const { return get_axis_pos_um(3); }

    protocol::CommandResult trigger_jog(uint8_t axis_id, int8_t dir, int32_t speed_um_s, int32_t distance_um);
    protocol::CommandResult trigger_home();
    // Fixture initialization only. NOT a homing procedure or sensor driver.
    protocol::CommandResult establish_reference_for_test(uint8_t axis_id, int32_t measured_um, bool measured_home_known);
    void invalidate_references();
    void abort_all();

    bool is_idle() const;

private:
    void step_axis(ProfileSegment& axis, uint32_t delta_us);

    std::array<ProfileSegment, protocol::AXIS_COUNT> axes_{};
};

} // namespace starz::firmware
