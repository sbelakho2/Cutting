#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace starz::measurement {
inline constexpr std::string_view revision="S1-B-MEAS-R1-2026-09-12";
namespace catalogue {
inline constexpr std::string_view wheel="8.0000.3123.0006";
// Kübler catalogue engineering/sources/kuebler-messrad-en.pdf, May 2026, pp.3–4, wheel No.123, bore06.
// Circumference is a catalogue free-wheel value, not loaded calibration.
inline constexpr double circumference_mm=100.0,circumference_tolerance_mm=0.1;
inline constexpr double wheel_od_mm=31.83,wheel_od_tolerance_mm=0.025;
inline constexpr double wheel_tread_mm=9.5,wheel_overall_mm=14.5;
inline constexpr double wheel_bore_mm=6.0,wheel_bore_h7_plus_mm=0.012;
inline constexpr double wheel_hub_od_mm=16.5,wheel_end_hub_od_mm=9.0;
inline constexpr double wheel_end_land_mm=0.75,wheel_screw_from_end_mm=2.5;
inline constexpr double wheel_tir_mm=0.2,wheel_mass_kg=0.016;
inline constexpr double wheel_shore_a=60;
inline constexpr std::string_view wheel_screw="M4x6 ISO4029";

inline constexpr std::string_view encoder="E40S6-1024-6-L-5";
inline constexpr std::uint32_t pulses_per_revolution=1024;
inline constexpr std::uint32_t counts_per_revolution=4*pulses_per_revolution;
inline constexpr double encoder_body_od_mm=40,encoder_body_length_mm=31;
inline constexpr double encoder_pilot_od_mm=20,encoder_pilot_minus_mm=0.02;
inline constexpr double encoder_pilot_length_mm=5,encoder_shaft_length_mm=15;
inline constexpr double encoder_shaft_min_mm=5.985,encoder_shaft_max_mm=5.990;
inline constexpr double encoder_flat_across_mm=5,encoder_flat_length_mm=10;
inline constexpr double encoder_mount_pcd_mm=30,encoder_mount_thread_depth_mm=5;
inline constexpr double encoder_starting_torque_nm=0.00392,encoder_inertia_kg_m2=4e-6;
inline constexpr double encoder_pulse_limit_hz=300000,encoder_speed_limit_rpm=5000;
inline constexpr double encoder_radial_limit_n=2*9.80665,encoder_axial_limit_n=9.80665;
inline constexpr double encoder_cable_d_mm=5,encoder_cable_length_mm=2000;

// The supplied coupling drawing defines these interfaces, but no hub length,
// screw tightening torque or minimum internal flexure clearance.
inline constexpr double coupling_od_mm=15,coupling_length_mm=22;
inline constexpr double coupling_bore_mm=6,coupling_bore_plus_mm=0.1;
inline constexpr double coupling_screw_from_end_mm=2.8;
inline constexpr double coupling_parallel_max_mm=0.25,coupling_angle_max_deg=5;
inline constexpr double coupling_axial_max_mm=0.5;
inline constexpr std::string_view bearing="SKF 626-2Z";
inline constexpr double bearing_d_mm=6,bearing_D_mm=19,bearing_width_mm=6;
inline constexpr double bearing_dynamic_n=2340,bearing_static_n=950;
inline constexpr double bearing_cn_min_um=2,bearing_cn_max_um=13;
}
namespace design {
inline constexpr double encoder_plate_width_mm=46,encoder_plate_thickness_mm=6;
inline constexpr double encoder_plate_pilot_mm=20.04,encoder_plate_outer_grid_mm=36;
inline constexpr double encoder_plate_encoder_clearance_mm=3.4,encoder_plate_support_clearance_mm=4.5;
inline constexpr double encoder_plate_edge_chamfer_mm=0.2;
// A chosen interpolation grid, not measured production calibration data.
inline constexpr std::size_t calibration_intervals=32;
inline constexpr std::uint32_t counts_per_calibration_interval=
    catalogue::counts_per_revolution/calibration_intervals;
static_assert(catalogue::counts_per_revolution%calibration_intervals==0);
// Selected metrology acceptance allocation, NOT demonstrated machine accuracy.
inline constexpr double calibration_absolute_allocation_mm=0.025;
inline constexpr double calibration_relative_allocation=0.0001;
}
}