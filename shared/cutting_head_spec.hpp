#pragma once
#include <array>
#include <numbers>

// S1-B parts-first cutting head. mm, N, s except explicitly named SI fields.
// This header is the common source for new CAD and engineering models.
// Source facts and design requirements are deliberately separate.
namespace starz::head {
inline constexpr char revision[] = "S1-B-KK86-R6-2026-09-12";
struct TorquePoint { double rpm, nm; };
namespace catalogue {
// Custom S1-B cut axis (docs/39): HGR20 rails + SFU1605 C7 ballscrew +
// BK12/BF12 supports on H005/H006 6082 plates; the carriage plate
// reproduces the KK86 interface so the CAD envelope constants are reused
// unchanged. Interface envelope values below are the required envelope the
// custom axis must reproduce; lead stays 10 mm for 100 pulses/mm.
inline constexpr char stage[] = "Custom S1-B cut axis HGR20/SFU1605-C7 (docs/39)";
inline constexpr double rail_length = 340, stage_length = 440, rail_width = 86;
inline constexpr double carriage_height = 46, screw_height = 32, lead = 10;
inline constexpr double useful_travel = 194, travel_reserve = 8;
inline constexpr double carriage_width = 60, carriage_pad_length = 75, carriage_total_length = 116;
inline constexpr double mounting_pitch = 46, input_diameter = 8, exposed_shaft = 18;
inline constexpr double cavity_diameter = 46, stage_mass_kg = 5.7;
inline constexpr double screw_dynamic_n = 7144, screw_static_n = 12642;
inline constexpr double guide_dynamic_n = 31458, guide_static_n = 50764;
inline constexpr double moment_roll_nm = 1507, moment_pitch_nm = 622, moment_yaw_nm = 622;
inline constexpr double bearing_static_n = 9240, bearing_max_n = 6320;
inline constexpr double start_torque_nm = 0.150, max_speed_mm_s = 740;
inline constexpr double repeatability_mm = 0.003, positioning_mm = 0.025;
inline constexpr char motor[] = "STEPPERONLINE 34E1K-85";
inline constexpr double motor_width = 85.85, motor_pitch = 69.58;
inline constexpr double motor_body = 112.3, encoder_section = 21.8;
inline constexpr double motor_rear = motor_body + encoder_section;
inline constexpr double motor_shaft = 37, motor_shaft_diameter = 14;
inline constexpr double motor_pilot = 73, motor_pilot_length = 1.6;
inline constexpr double motor_shaft_length_tolerance = 1;
inline constexpr double motor_pilot_min = 72.95, motor_pilot_max = 73;
inline constexpr double motor_pilot_length_max = 1.8;
inline constexpr double rotor_inertia_kg_m2 = 3600e-7;
inline constexpr double motor_resistance_ohm = 0.54, motor_inductance_h = 0.005;
inline constexpr double motor_holding_nm = 8.5, curve_rms_a = 5.7, curve_voltage = 60;
// Visually digitised catalogue graph, not manufacturer's tabulated guarantees.
// Quantisation/read-off allowance is applied separately. No extrapolation below50rpm.
inline constexpr std::array<TorquePoint,16> torque_curve{{
    {50,7.65},{100,7.30},{200,5.90},{300,4.55},{400,3.60},{500,2.85},
    {600,2.30},{700,2.00},{800,1.75},{900,1.45},{1000,1.35},{1100,1.10},
    {1200,0.95},{1300,0.80},{1400,0.93},{1500,0.78}
}};
inline constexpr double curve_reading_allowance_nm = 0.10;
inline constexpr char coupling[] = "R+W MK2/100/50/8/14";
inline constexpr double coupling_diameter = 40, coupling_length = 50, coupling_fit = 16;
inline constexpr double coupling_length_min = 49, coupling_length_max = 50;
inline constexpr double coupling_torque_nm = 10, coupling_stiffness_nm_rad = 9050;
inline constexpr double coupling_inertia_kg_m2 = 160e-7;
inline constexpr double coupling_mass_kg = 0.120;
inline constexpr double supply_voltage = 60, supply_current_a = 5.8;
inline constexpr double driver_max_voltage = 110, driver_max_ambient_c = 40;
}
namespace design {
inline constexpr double datum_width = 180, datum_length = 1040, datum_thickness = 20;
inline constexpr double riser_width = 110, riser_length = 355, riser_height = 20;
inline constexpr double stage_base_z = datum_thickness + riser_height;
inline constexpr double carriage_z = stage_base_z + catalogue::carriage_height;
inline constexpr double wire_z = carriage_z + 26;
inline constexpr double stage_distal_y = -15;
inline constexpr double stage_front_y = stage_distal_y - catalogue::stage_length;
inline constexpr double rail_start_y = stage_front_y + 87;
inline constexpr double rail_end_y = stage_distal_y - 13;
inline constexpr double carriage_open_y = -102;
inline constexpr double blade_open_apex = 8, blade_cut_travel = 4.1, blade_limit = 4.2;
inline constexpr double blade_face_gap = 0.020, blade_thickness = 3; // face gap is an ADJUSTED-AT-ASSEMBLY target (shim/lap per docs/36 P8), not a plain-plate machining tolerance
inline constexpr double adapter_thickness = 15, adapter_width = 100;
inline constexpr double adapter_thickness_tolerance = 0.02;
inline constexpr double adapter_motor_pilot_diameter = 73.10, adapter_motor_pilot_depth = 2.10;
inline constexpr double adapter_f0_register_diameter = 49.97, adapter_f0_register_length = 2;
inline constexpr double adapter_f0_counterbore_depth = 8.5;
inline constexpr double stage_hub_shoulder_gap = 1.0;
inline constexpr double stage_shaft_tip = 36, stage_shaft_shoulder = 54;
inline constexpr double coupling_far = stage_shaft_shoulder - stage_hub_shoulder_gap;
inline constexpr double coupling_near = coupling_far - catalogue::coupling_length;
inline constexpr double motor_face_y = stage_front_y - adapter_thickness;
inline constexpr double overall_head_half_y = -motor_face_y + catalogue::motor_rear;
inline constexpr double motor_bay_half_width = 160; // Cable/service reservation, not motor material.
inline constexpr double holder_foot_width = 60, holder_foot_length = 75, holder_foot_thickness = 12;
inline constexpr double holder_arm_width = 70, holder_arm_length = 115, holder_arm_height = 44;
inline constexpr double holder_arm_x = -35, holder_arm_y = -30, blade_seat_x = -3.01;
inline constexpr double blade_pocket_y = 60, blade_pocket_fillet = 3;
inline constexpr double blade_mount_y = 74, blade_apex_y = 98;
inline constexpr double blade_speed_mm_s = 12.5, blade_accel_mm_s2 = 300;
inline constexpr int pulses_per_revolution = 1000;
inline constexpr double steps_per_mm = pulses_per_revolution / catalogue::lead;
// Explicit acceptance loads and qualification factors, NOT measured wire data.
inline constexpr double working_thrust_n = 2500, transverse_load_n = 750, vertical_load_n = 375;
inline constexpr double proof_load_n = 5000;
inline constexpr double efficiency_lower = 0.90;
inline constexpr double running_curve_fraction = 0.80; // Qualification reserve, not guaranteed hot torque.
inline constexpr double minimum_running_margin = 1.25;
inline constexpr double carriage_payload_limit_kg = catalogue::stage_mass_kg + 5.0;
inline constexpr double structural_young_pa = 205e9, poisson = 0.29, density_kg_m3 = 7850;
inline constexpr double specified_yield_pa = 650e6; // Material certificate requirement, 42CrMo4+QT.
inline constexpr double structural_factor = 3;
}
static_assert(design::coupling_far <= design::stage_shaft_shoulder);
static_assert(design::coupling_far - catalogue::coupling_fit > design::stage_shaft_tip);
static_assert(-design::adapter_thickness + catalogue::motor_shaft >
              design::coupling_near + catalogue::coupling_fit);
static_assert(catalogue::coupling_diameter < catalogue::cavity_diameter);
}