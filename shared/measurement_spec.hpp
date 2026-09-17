#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace starz::measurement {
inline constexpr std::string_view revision="S1-B-MEAS-R2-2026-09-16";
namespace catalogue {
inline constexpr std::string_view wheel="E60137";
// ifm E60137 measuring wheel "U:200 N:06", datasheet E60137-01 EN-GB
// (engineering/sources/ifm-e60137-datasheet-en.txt, read 2026-09-16):
// circumference 200 mm, OD 63.66 mm, bore 6 mm, aluminium wheel with PU
// (polyurethane) surface 90 Shore A, 64.5 g. Selected at automation24 item
// 102756, EUR 57.00 ex VAT, in stock, dispatch next working day. ifm publishes
// no circumference/OD tolerance and no hub or clamping-screw dimensions;
// the 0.1% tolerance and the retained profile stations below are assumed
// placeholders pending the manufacturer drawing at incoming inspection.
inline constexpr double circumference_mm=200.0,circumference_tolerance_mm=0.2;
inline constexpr double wheel_od_mm=63.66,wheel_od_tolerance_mm=0.1;
inline constexpr double wheel_tread_mm=9.5,wheel_overall_mm=14.5;
inline constexpr double wheel_bore_mm=6.0,wheel_bore_h7_plus_mm=0.012;
inline constexpr double wheel_hub_od_mm=16.5,wheel_end_hub_od_mm=9.0;
inline constexpr double wheel_end_land_mm=0.75,wheel_screw_from_end_mm=2.5;
inline constexpr double wheel_tir_mm=0.2,wheel_mass_kg=0.0645;
inline constexpr double wheel_shore_a=90;
inline constexpr std::string_view wheel_screw="M4x6 radial clamp envelope;ifm hub screw detail not published";

inline constexpr std::string_view encoder="RU3100";
// ifm RU3100 incremental encoder, basic line, magnetic, datasheet RU3100-01
// (archived engineering/sources/ifm-ru3100-datasheet.pdf, DE, 2023-04-13;
// EN-GB text read 2026-09-16): solid shaft 6 mm x 10 mm, no flat, body
// 58.6 mm x 63.5 mm, synchro-flange fixing with 3 x M4 x 0.7 tapped 6 mm
// deep on PCD 42 (drawing front view labels only 42 and 58.6; the
// 36.5-class flange seat is the plate-side centering per the ifm mounting
// practice - confirm the centering feature at receipt). 4.75...30 VDC,
// HTL/TTL push-pull, factory 1024 PPR, 1000 kHz switching frequency,
// 12000 rpm, starting torque <= 0.01 Nm, M12 5-pole radial/axial connector.
// Resolution is programmable 1...10000 via IO-Link. 2026-09-17: the encoder
// is parameterised to 2048 PPR via IO-Link (docs/13 step 9, docs/42) so the
// 200 mm wheel count spacing halves from the 1024 factory value to
// 0.024414 mm/count; this IS a commissioning step, not a factory state.
// Selected at automation24 item 102717,
// EUR 186.40 ex VAT, in stock, dispatch next working day.
inline constexpr std::uint32_t pulses_per_revolution=2048;
inline constexpr std::uint32_t counts_per_revolution=4*pulses_per_revolution;
inline constexpr double encoder_body_od_mm=58.6,encoder_body_length_mm=63.5;
inline constexpr double encoder_shaft_length_mm=10;
// ifm publishes no shaft-diameter tolerance (the archived datasheet states
// only "Shaft diameter [mm] 6"); 5.99-6.01 is an assumed incoming-inspection
// placeholder, like the retained wheel-profile stations below.
inline constexpr double encoder_shaft_min_mm=5.99,encoder_shaft_max_mm=6.01;
inline constexpr double encoder_mount_pcd_mm=42,encoder_mount_thread_depth_mm=6;
inline constexpr double encoder_starting_torque_nm=0.01,encoder_inertia_kg_m2=4e-6;
inline constexpr double encoder_pulse_limit_hz=1000000,encoder_speed_limit_rpm=12000;
inline constexpr double encoder_radial_limit_n=60,encoder_axial_limit_n=40;
inline constexpr double encoder_cable_d_mm=6,encoder_cable_length_mm=2000;

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
inline constexpr double encoder_plate_width_mm=60,encoder_plate_thickness_mm=6;
// 36.5-class synchro-flange seat bore (ifm mounting practice; the encoder
// envelope models no pilot because the archived drawing labels none).
inline constexpr double encoder_plate_seat_mm=36.5,encoder_plate_outer_grid_mm=36;
inline constexpr double encoder_plate_encoder_clearance_mm=4.5;
// Tapped M4 support holes are modelled at the thread major diameter per the
// repository convention (screw shanks at major-0.02 leave 0.01 clearance).
inline constexpr double encoder_plate_support_tap_mm=4.0;
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
