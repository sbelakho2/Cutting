#pragma once
#include <cstdint>
#include "cutting_head_spec.hpp"

// S1-B controls reference. CAD millimetres; controller integer micrometres.
// Altering mechanical datums requires regeneration, calibration and requalification.
namespace starz::spec {
inline constexpr char revision[] = "S1-B-controls-reference-2026-09-11";
// Full-machine layout constants below remain historical transport references.
// The active cutting head uses cutting_head_spec.hpp; legacy CAD is opt-in only.
inline constexpr double base_x_mm = 800.0;
inline constexpr double base_y_mm = 700.0;
inline constexpr double base_thickness_mm = 8.0;
inline constexpr double wire_x_mm = 400.0;
inline constexpr double wire_y_mm = 300.0;
inline constexpr double wire_z_mm = 100.0;
inline constexpr double encoder_offset_mm = 50.0;
inline constexpr double feed_near_mm = 90.0;
inline constexpr double feed_far_mm = 140.0;
inline constexpr double roller_diameter_mm = 40.0;
inline constexpr double wheel_circumference_mm = 200.0; // ifm E60137 nominal 200 mm, not calibration!
inline constexpr std::int32_t encoder_counts_rev = 4096; // 1024 PPR, quadrature x4.
inline constexpr std::int32_t nominal_encoder_nm_count = 48828;
inline constexpr std::int32_t min_length_um = 200000;
inline constexpr std::int32_t max_length_um = 3000000;
inline constexpr std::int32_t min_strip_um = 3000;
inline constexpr std::int32_t max_strip_um = 25000;
inline constexpr std::int32_t max_feed_um_s = 500000;
inline constexpr std::int32_t feed_accel_um_s2 = 1500000;
inline constexpr std::int32_t strip_speed_um_s = 80000;
inline constexpr std::int32_t strip_accel_um_s2 = 400000;
inline constexpr std::int32_t blade_speed_um_s = static_cast<std::int32_t>(head::design::blade_speed_mm_s * 1000);
inline constexpr std::int32_t blade_accel_um_s2 = 300000;
inline constexpr std::int32_t blade_open_apex_um = 8000;
inline constexpr std::int32_t blade_cut_position_um = 4100; // 0.20 mm apex overlap.
inline constexpr std::int32_t blade_max_position_um = 4200;
inline constexpr std::int32_t strip_clear_um = 3000;
inline constexpr std::int32_t stock_clear_um = 10000;
inline constexpr std::int32_t ejection_um = 230000;
inline constexpr std::uint64_t heartbeat_timeout_us = 750000;
inline constexpr std::uint64_t move_timeout_us = 30000000;
inline constexpr std::uint32_t max_quantity = 1000000;
inline constexpr double feed_steps_mm = 3200.0 / (3.14159265358979323846 * roller_diameter_mm);
inline constexpr double blade_steps_mm = head::design::steps_per_mm; // 1000 pulses / actual10mm lead.
inline constexpr std::uint32_t pio_clock_hz = 1000000;
inline constexpr std::uint32_t max_step_hz = 40000;
}