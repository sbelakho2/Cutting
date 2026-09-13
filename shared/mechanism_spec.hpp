#pragma once
#include <string_view>

// Mechanism integration authority: pressure carriers, guide posts, wire
// guides, measuring-wheel support and frame. Coordinates are machine
// coordinates (wire along X at Y=0, Z=112; RIGHT feed at +X). LEFT parts are
// the RIGHT parts rotated 180 degrees about Z, as the feed cassettes are.
namespace starz::mechanism {
inline constexpr std::string_view revision="S1-B-MECH-R3-2026-09-13";
namespace catalogue {
// MISUMI UK web catalogue, read 2026-09-13, prices EUR ex-VAT, Frankfurt stock.
inline constexpr std::string_view bushing="LHFRW12";
inline constexpr double bushing_dr_mm=12,bushing_D_mm=21,bushing_D_minus_mm=0.016;
inline constexpr double bushing_L_mm=57,bushing_flange_d_mm=42,bushing_flange_t_mm=6;
inline constexpr double bushing_pcd_mm=32,bushing_hole_mm=4.5,bushing_cbore_mm=7.5,bushing_cbore_t_mm=4.1;
inline constexpr double bushing_C_n=657,bushing_C0_n=1200,bushing_moment_nm=10.9,bushing_mass_kg=0.110;
inline constexpr double bushing_eur=14.58;
inline constexpr std::string_view post="PSFJ12-240";
inline constexpr double post_d_mm=12,post_L_mm=240,post_eur=11.70;
inline constexpr std::string_view carrier_spring="SWU17-60";
inline constexpr double carrier_spring_D_mm=17,carrier_spring_d_mm=10.5,carrier_spring_L_mm=60;
inline constexpr double carrier_spring_rate_n_mm=4.09,carrier_spring_load_n=147.1,carrier_spring_deflection_mm=36;
inline constexpr double carrier_spring_eur=3.53;
inline constexpr std::string_view arm_spring="SWU10.5-35";
inline constexpr double arm_spring_D_mm=10.5,arm_spring_d_mm=6.0,arm_spring_L_mm=35;
inline constexpr double arm_spring_rate_n_mm=3.27,arm_spring_load_n=68.6,arm_spring_deflection_mm=21;
inline constexpr double arm_spring_eur=1.80;
}
namespace design {
// F201 lower cassette base (6082-T6): full-height block plus a flange over
// the head datum/riser. Bed top face is Z=0.
inline constexpr double base_x_min=47,base_block_x_min=96,base_x_max=168;
inline constexpr double base_y_min=-130,base_y_max=55;
inline constexpr double base_top_z=55.75,base_flange_bottom_z=42;
inline constexpr double post_x=108,post_rear_y=-106,post_front_y=32,post_bore_depth=30;
inline constexpr double post_clamp_z=28,post_clamp_rear_y=-124,post_clamp_front_y=48;
inline constexpr double lower_screw_cbore_z=48;
inline constexpr double bed_screw_x1=126,bed_screw_x2=158,bed_screw_y1=-112,bed_screw_y2=40;
// F202 upper pressure carrier (6082-T6) sits on the two upper housing bases.
inline constexpr double carrier_t=10;
inline constexpr double spring_x=108,spring_y=0;
// F203 bridge (C45 flat 40x12) clamped to the post tops.
inline constexpr double bridge_x_min=88,bridge_x_max=128,bridge_y_min=-126,bridge_y_max=52;
inline constexpr double bridge_z_min=236,bridge_z_max=248;
inline constexpr double bridge_clamp_rear_y=-118,bridge_clamp_front_y=44;
// Nip preload target: feed spec roller_radial_design_n per nip.
inline constexpr double nip_force_target_n=60,nip_force_min_n=50,nip_force_max_n=75;
inline constexpr double cap_t=6,cap_d=24,cap_spigot_d=10,cap_spigot_L=10,cap_recess_d=9,cap_recess_t=2;
// F205 wire guide holder and F206 brass insert in the head/feed X gap.
inline constexpr double guide_x_min=38,guide_x_max=47,guide_half_y=12,guide_z_min=42,guide_z_max=125;
inline constexpr double guide_screw_y=6,guide_screw_z=49;
inline constexpr double insert_d=8,insert_bore_clearance=0.5,insert_cone_depth=1.5;
// Measuring wheel module (LEFT infeed side, wheel axis along Y at X=-205).
inline constexpr double meas_x=-205;
inline constexpr double wheel_front_y=-4.75;
inline constexpr double housing_x_min=-228,housing_x_max=-182,housing_y_min=10.5,housing_y_max=70,housing_half=23;
inline constexpr double front_bearing_y=12,rear_bearing_y=30,coupling_y=44,plate_y=70;
inline constexpr double shaft_y_min=-5.75,shaft_y_max=52;
inline constexpr double arm_x_min=-283,arm_x_max=-182,arm_y_min=20,arm_y_max=60,arm_t=12,arm_hub_d=30;
inline constexpr double pivot_x=-266,arm_spring_x=-241;
inline constexpr double column_x_min=-290,column_x_max=-250,column_y_min=10,column_y_max=80;
inline constexpr double column_slot_y_min=19.5,column_slot_y_max=60.5,column_slot_z=138;
inline constexpr double lug_z_min=204,lug_z_max=216,lug_x_max=-232;
inline constexpr double bracket_base_x_min=-306,bracket_base_x_max=-235,bracket_base_y_min=-16,bracket_base_y_max=96,bracket_base_t=12;
inline constexpr double bracket_screw_inset=8;
inline constexpr double wheel_force_target_n=10,wheel_force_min_n=6,wheel_force_max_n=19;
inline constexpr double roller_column_x_min=-232,roller_column_x_max=-216,roller_column_half_y=10,roller_column_z_max=100;
inline constexpr double roller_base_x_min=-234,roller_base_x_max=-198,roller_base_half_y=24,roller_base_t=10;
inline constexpr double roller_screw_x1=-226,roller_screw_x2=-206,roller_screw_y=16;
inline constexpr double fork_x_min=-216,fork_x_max=-198,fork_half_y=9.5,fork_z_min=80,fork_z_max=108,fork_slot_half=3.2,fork_slot_z=86;
inline constexpr double fork_screw_y=6,fork_screw_z=90,fork_slot_travel=4;
// Frame: 6082 tooling-plate bed on a welded S235JR 60x60x3 tube frame.
inline constexpr double bed_half_x=320,bed_half_y=560,bed_t=25;
inline constexpr double tube=60,tube_wall=3,rail_x=250,rail_y=510,leg_length=760,foot=100,foot_t=8;
// R2 guard/payoff selections (docs/26): DC01 2 mm steel side/rear/roof
// panels, 5 mm polycarbonate front door, Ø20 h6 C45 payoff spindle on two
// 6204-2Z/C3 pockets, Ø400 x 160 welded spool and a friction brake disc with
// an adjustable caliper bracket. Still no safety-gap or load qualification.
inline constexpr double guard_inner_half_x=345,guard_wall_t=2,guard_door_t=5;
inline constexpr double guard_side_y_min=-625,guard_side_y_max=625;
inline constexpr double guard_front_y_min=-630,guard_front_y_max=-625;
inline constexpr double guard_rear_y_min=628,guard_rear_y_max=630;
inline constexpr double guard_wall_z_min=0,guard_wall_z_max=315;
inline constexpr double guard_roof_z_min=318,guard_roof_z_max=320;
inline constexpr double guard_wire_passage_d=30;
// Automation24 pages observed 2026-09-13 list the selected SLC body as
// 30 x 192 x 44 mm and ACF-1 actuator as 60 x 40 x 52.6 mm. Their orientation,
// the surrounding pockets, fixed-jamb datum and static separation below are
// only CAD accommodation envelopes; manufacturer mounting geometry, actuation
// engagement and a safety function are not established.
inline constexpr double guard_lock_body_x=30,guard_lock_body_y=44,guard_lock_body_z=192;
inline constexpr double guard_lock_plate_x_min=-345,guard_lock_plate_x_max=-285;
inline constexpr double guard_lock_plate_y_min=-690,guard_lock_plate_y_max=-635;
inline constexpr double guard_lock_plate_z_min=40,guard_lock_plate_z_max=270;
inline constexpr double guard_lock_body_x_min=-330,guard_lock_body_y_min=-684.5,guard_lock_body_z_min=55.5;
inline constexpr double guard_lock_pocket_clearance=0.5;
inline constexpr double guard_actuator_x=60,guard_actuator_y=52.6,guard_actuator_z=40;
inline constexpr double guard_actuator_x_min=-344.5,guard_actuator_y_min=-624.5,guard_actuator_z_min=130;
// Payoff R2 (docs/26): selected design, not fabricated and not load-qualified.
inline constexpr double payoff_spool_x=-650,payoff_spool_y_min=-80,payoff_spool_z=112;
inline constexpr double payoff_spool_r=200,payoff_flange_t=3,payoff_spool_y_length=160;
inline constexpr double payoff_hub_r=30,payoff_hub_wall=2.5;
inline constexpr double payoff_shaft_r=10,payoff_shaft_clearance=0.25;
inline constexpr double payoff_shaft_y_min=-330,payoff_shaft_y_max=330;
inline constexpr double payoff_bearing_r=23.5,payoff_bearing_w=14;
inline constexpr double payoff_disc_r=90,payoff_disc_t=6,payoff_disc_y_min=295;
inline constexpr double payoff_caliper_x_min=-700,payoff_caliper_x_max=-600;
inline constexpr double payoff_caliper_y_min=308,payoff_caliper_y_max=328;
inline constexpr double payoff_caliper_z_min=172,payoff_caliper_z_max=212;
inline constexpr double payoff_wire_x_min=-440,payoff_wire_x_max=440;
inline constexpr double payoff_base_x_min=-900,payoff_base_x_max=-470,payoff_base_y_min=-280,payoff_base_y_max=280;
inline constexpr double payoff_base_z_min=-853,payoff_base_z_max=-845;
inline constexpr double payoff_post_t=18;
inline constexpr double payoff_post_x_min=-685,payoff_post_x_max=-615;
inline constexpr double payoff_post_y1_inner=-271,payoff_post_y1_outer=-289;
inline constexpr double payoff_post_y2_inner=271,payoff_post_y2_outer=289;
inline constexpr double payoff_post_z_min=-845,payoff_post_z_max=320;
}
}
