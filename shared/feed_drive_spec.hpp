#pragma once
#include <array>
#include <string_view>

namespace starz::feed {
inline constexpr std::string_view revision="S1-B-FEED-R6-2026-09-12";
namespace catalogue {
inline constexpr std::string_view motor="24HS40-5004D-E1000";
inline constexpr std::string_view driver="CL57T-V41";
inline constexpr std::string_view supply="LE-350-36";
inline constexpr std::string_view pulley="HTPA20S5M250-A-N14";
inline constexpr std::string_view belt="HTBN225S5M-250";
inline constexpr std::string_view roller="UMCL40-10-20-MC";
inline constexpr std::string_view coupling="MK2/100/50/10/12";
inline constexpr double motor_shaft_d=10, motor_shaft_min=20, motor_shaft_max=22;
inline constexpr double motor_register=38.1, motor_register_tolerance=0.05;
inline constexpr double motor_grid=50, motor_grid_tolerance=0.25, motor_hole=5.1;
inline constexpr double roller_d=40, roller_bore=10, roller_width=20, collar_radial_t=4;
// Currentproductdrawing labelsH7;archivedEUcatalogue shows+0.01/+0.03.
// One incoming acceptance interval covers both for the selected10mm SKU.
inline constexpr double roller_bore_min=10.000,roller_bore_max=10.030;
inline constexpr double pulley_teeth=20, pitch_mm=5, pulley_width=32;
inline constexpr double pulley_od=30.87, pulley_flange_d=36, pulley_bore=14;
inline constexpr double pulley_key_width=5, pulley_key_depth=2.3;
inline constexpr double belt_length=225, belt_width=25, belt_length_tolerance=0.41;
inline constexpr double width_factor=2.84, engagement_factor=1.0;
inline constexpr double tension_recommended_n=166, tension_max_n=221, tension_y_n=151;
inline constexpr double minimum_adjust_in=10, minimum_adjust_out=2;
// MISUMI Table39, S5M,20 teeth,10mm reference width. These are printed table
// entries, NOT motor-curve digitisation. No extrapolation below50rpm is allowed.
inline constexpr std::array<double,16> power_rpm{50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800};
inline constexpr std::array<double,16> power_w{25,46,66,84,101,118,134,150,165,180,195,209,223,237,250,264};
// Rejected25-tooth S3M/10mm combination, Table38,6mm reference width.
inline constexpr std::array<double,5> rejected_rpm{50,100,150,200,250};
inline constexpr std::array<double,5> rejected_power_w{9,16,23,29,35};
inline constexpr double rejected_width_factor=1.79;
inline constexpr double rear_bearing_d=12,rear_bearing_D=28,rear_bearing_B=8;
inline constexpr double front_bearing_d=15,front_bearing_D=32,front_bearing_B=9;
inline constexpr double rear_bearing_C=5400,rear_bearing_C0=2360;
inline constexpr double front_bearing_C=5850,front_bearing_C0=2850;
inline constexpr double bearing_bore_minus=0.007,bearing_width_minus=0.060;
// A12-class shaft ring (2026-09-16 selection: Wegertseder Art. 1950-010
// DIN 471 12x1 Federstahl, TR-RR01; same groove geometry, no CAD change).
inline constexpr double seeger_a12_groove_d=11.5,seeger_a12_groove_minus=0.11;
inline constexpr double seeger_a12_width=1,seeger_a12_width_minus=0.06;
inline constexpr double seeger_a12_groove_width_min=1.1;
inline constexpr double seeger_a12_groove_capacity_n=1500;
inline constexpr double seeger_a12_ring_capacity_n=5000;
}
namespace design {
// Limits below are design/inspection settings, not measured process forces.
inline constexpr double nominal_centres=62.5,min_centres=52.5,max_centres=64.5;
inline constexpr double feed_limit_m_s=0.50,shaft_limit_rpm=260;
inline constexpr double running_torque_nm=1.0,reversal_service_factor=2.5;
inline constexpr double mechanical_fault_torque_nm=5.0;
inline constexpr double tension_target_n=190,tension_min_n=180,tension_max_n=200;
inline constexpr double belt_radial_design_n=480,roller_radial_design_n=60;
inline constexpr double steel_E_mpa=205000,steel_yield_mpa=650;
// Shared rotor stations, millimetres from the rear12mm shaft end.
inline constexpr double rear_support=26.5,front_support=77.5;
inline constexpr double pulley_station=47.5,roller_station=103.5;
// MC access pockets are M+1=Ø4 in the urethane,at roller mid-width.
// Put the wire on the solid tread band,5.25mm toward the front collar face.
inline constexpr double wire_contact_station=108.75,wire_diameter_min=1.2,wire_diameter_max=4.5;
inline constexpr double wire_axis_z=112,wire_lane_tolerance=0.20,mc_centre_tolerance=0.15;
inline constexpr double shaft_end=113.0;
inline constexpr double groove_start=18.5,groove_end=19.62,groove_d=11.45;
inline constexpr double groove_root_radius=0.04,ring_installed_start=18.56;
inline constexpr double bearing_shoulder_radius=0.20,neck_shoulder_radius=0.50;
inline constexpr double key_floor_radius=0.15,key_inside_chamfer=0.20;
inline constexpr double washer_slot_start=31.20;
inline constexpr double washer_slot_end=73.00,washer_slot_width=4.20,washer_slot_floor=5.50;
inline constexpr double washer_slot_plan_radius=0.50,washer_slot_intersection_radius=0.50;
inline constexpr double washer_slot_floor_radius=0.15;
inline constexpr double shaft_end_chamfer=0.20,thread_lead_chamfer=0.50;
inline constexpr double thread_relief_start=71.50,thread_relief_end=73.00,thread_relief_d=13.50;
inline constexpr double retaining_entry_chamfer=0.20,retaining_full_thread_depth=14.00;
inline constexpr double retaining_drill_d=4.20,retaining_drill_full_depth=18.00,retaining_drill_point_degrees=118.0;
inline constexpr double rear_bearing_start=22.5,pulley_start=31.5;
inline constexpr double thread_start=64.5,front_bearing_start=73;
inline constexpr double shoulder_start=82,shoulder_end=87,roller_shaft_start=92;
inline constexpr double roller_start=93.5,roller_end=113.5;
inline constexpr double pulley_key_length=28,roller_key_length=18;
inline constexpr double roller_key_width=3,roller_key_height=4;
inline constexpr double roller_key_shaft_depth=1.8,roller_key_hub_depth=2.35;
inline constexpr double motor_face=-35.2,coupling_start=-33,coupling_end=17;
inline constexpr double near_nip_x=75.5;
inline constexpr double minimum_finished_length=200,maximum_strip_length=25,minimum_insulated_grip_margin=10;
inline constexpr double front_module_start=72.98,front_module_bottom=-18.5;
inline constexpr double module_bolt_x=16,module_bolt_station=84;
inline constexpr double module_pin_x=10,module_pin_station=89;
inline constexpr double rear_mount_station=25,front_mount_station=65;
inline constexpr double rear_housing_bore=28.013,front_housing_bore=32.0135;
}
}