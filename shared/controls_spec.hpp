#pragma once
// STARZ S1-B controls specification, R1. Parts first: every number here is
// derived from a selected, catalogue-verified part (bom/selected-controls.csv,
// bom/selected-cutting-head.csv, bom/selected-transport-and-cabinet.csv) or the
// shared mechanical specifications. Nothing here has been energised or measured
// on a machine; the physical qualification record (docs/20) stays authoritative.
#include <cstdint>
#include "machine_spec.hpp"
#include "feed_drive_spec.hpp"
#include "cutting_head_spec.hpp"

namespace starz::ctrl {
inline constexpr char revision[] = "S1-B-CTRL-R1-2026-09-13";

// ---------------------------------------------------------------- controller
namespace mcu {
inline constexpr char part[] = "Raspberry Pi Pico 2 SC1631 (RP2350)";
inline constexpr std::uint32_t sys_clock_hz = 150000000;   // RP2350 default.
inline constexpr std::uint32_t gpio_count = 26;            // GP0-22, GP26-28.
inline constexpr bool gpio_5v_tolerant = false;            // all board interfaces terminate at 3.3 V.
inline constexpr std::uint32_t watchdog_timeout_ms = 100;  // hardware watchdog reload period budget.
inline constexpr std::uint32_t control_period_us = 1000;   // 1 kHz supervisory loop.
}

// ------------------------------------------------------------ GPIO assignment
// GP0-7 feed 3.3 V-input channels of two TPL7407LDR NMOS arrays. Every selected
// driver is wired common-anode: PUL+/DIR+/ENA+ are +24V_PELV and the NMOS outputs
// sink the corresponding negative input. CL57T S3 and CL86T 5/24V selectors are
// set to 24 V. Every PUL pin is a PIO "set" pin and every DIR pin is a PIO
// "side-set" pin. The selected drivers require a >=1 us pulse and >=2 us DIR lead.
namespace pin {
inline constexpr std::uint8_t blade_front_pul = 0, blade_front_dir = 1;
inline constexpr std::uint8_t blade_rear_pul  = 2, blade_rear_dir  = 3;
inline constexpr std::uint8_t feed_left_pul   = 4, feed_left_dir   = 5;
inline constexpr std::uint8_t feed_right_pul  = 6, feed_right_dir  = 7;
// Measuring encoders ifm RU3100 (TTL/HTL push-pull, 4.75-30 VDC): the
// 2026-09-16 substitution from the Autonics 5 V RS-422 line driver keeps the
// A/B capture pins. Reviewed 2026-09-16: the encoder must be parameterised
// to TTL mode via IO-Link (factory setting is HTL 50 mA) and, 2026-09-17,
// to 2048 PPR (factory 1024, programmable 1...10000) per docs/13 step 9;
// its single-ended
// A/B outputs drive the AM26LV32EIDR + inputs while the - inputs sit on a
// 2.5 V mid-bias, and the four 120 ohm RS-422 terminators are not fitted.
// Wiring note in docs/22 §6/§7.
inline constexpr std::uint8_t enc_left_a = 8,  enc_left_b = 9;
inline constexpr std::uint8_t enc_right_a = 10, enc_right_b = 11;
// 24 V field loops through Phoenix PLC-RSC-24DC/21 relay couplers; the coupler
// contact closes the GPIO to 0 V (internal pull-up), so LOW = loop energised.
inline constexpr std::uint8_t in_start     = 12;  // 3SU1152-0AB40-1BA0 NO: LOW while pressed.
inline constexpr std::uint8_t in_stop_ok   = 13;  // 3SU1152-0AB20-1CA0 NC: LOW while NOT pressed (loop closed).
inline constexpr std::uint8_t in_reset     = 14;  // 3SU1152-0AB50-1BA0 NO: LOW while pressed.
inline constexpr std::uint8_t in_safety_ok = 15;  // XPSUAF13AP enabling contact 13-14: LOW while E-stop AND guard circuit closed.
inline constexpr std::uint8_t in_home_rear = 16;  // IE5338 on blade-rear carriage (PNP 24 V -> coupler): LOW at open end.
// GP17 is a TPL7407LDR input. Its shared low-side output connects all ENA- pins;
// GPIO HIGH makes ENA effective HIGH and enables all drivers. A 100 kOhm board
// pull-down keeps the channel off (all drives disabled) through reset/power loss.
inline constexpr std::uint8_t out_drv_enable = 17;
// Each driver ALM output energises a dedicated 24 V Phoenix PLC-RSC relay coil;
// its isolated NO contact closes the GPIO to 0 V on alarm. This meets the
// selected drivers' 5–24 V / 100 mA alarm-interface requirement: LOW = alarm.
inline constexpr std::uint8_t in_alm_blade_front = 18, in_alm_blade_rear = 19;
inline constexpr std::uint8_t in_alm_feed_left = 20,  in_alm_feed_right = 21;
// 24 V indicators through TPL7407LDR low-side channels: HIGH = on.
inline constexpr std::uint8_t out_led_green = 22;  // in START button.
inline constexpr std::uint8_t out_led_red   = 26;  // in STOP button.
inline constexpr std::uint8_t in_home_front = 27;  // IE5338 on blade-front carriage (PNP 24 V -> coupler): LOW at open end.
inline constexpr std::uint8_t out_buzzer    = 28;  // 3SU1200-6LB10-1AA0.
// The blue LED in the RESET button is wired directly across the 24 V control
// supply ("control voltage present"); it is not a GPIO.
}

// -------------------------------------------------------------- axis scaling
namespace axis {
enum Id : std::uint8_t { blade_front = 0, blade_rear = 1, feed_left = 2, feed_right = 3, count = 4 };

// Blade axes: CL86T-V41 at 1000 pulses/rev (bom interface), KK86 lead 10 mm.
inline constexpr double blade_steps_per_mm = head::design::steps_per_mm;          // 100
inline constexpr double blade_speed_mm_s   = head::design::blade_speed_mm_s;      // 12.5
inline constexpr double blade_accel_mm_s2  = head::design::blade_accel_mm_s2;     // 300
inline constexpr double blade_open_mm      = head::design::blade_open_apex;       // 8.0 (home, open)
inline constexpr double blade_cut_mm       = head::design::blade_cut_travel;      // 4.1 (sever)
inline constexpr double blade_limit_mm     = head::design::blade_limit;           // 4.2 (software limit)

// Feed axes: CL57T-V41 at 2000 pulses/rev (bom "2000 pulses"), motor coupled
// 1:1 to the roller shaft (MK2 coupling), 20T:20T belt to the mating roller,
// roller Ø40 (UMCL40).
inline constexpr double feed_pulses_per_rev = 2000.0;
inline constexpr double feed_roller_circumference_mm = 3.14159265358979323846 * feed::catalogue::roller_d; // 125.664
inline constexpr double feed_steps_per_mm = feed_pulses_per_rev / feed_roller_circumference_mm;             // 15.915
inline constexpr double feed_speed_mm_s   = feed::design::feed_limit_m_s * 1000.0;                          // 500
inline constexpr double feed_accel_mm_s2  = spec::feed_accel_um_s2 / 1000.0;                                // 1500
inline constexpr double feed_shaft_limit_rpm = feed::design::shaft_limit_rpm;                               // 260

inline constexpr double strip_speed_mm_s = spec::strip_speed_um_s / 1000.0;   // 80
inline constexpr double strip_accel_mm_s2 = spec::strip_accel_um_s2 / 1000.0; // 400

// Highest step rate any state machine must generate.
inline constexpr std::uint32_t max_step_hz = 8000; // 500 mm/s * 15.915 = 7958 Hz.
static_assert(feed_speed_mm_s * feed_steps_per_mm < max_step_hz);
static_assert(feed_speed_mm_s / feed_roller_circumference_mm * 60.0 < feed_shaft_limit_rpm); // 238.7 rpm
static_assert(blade_speed_mm_s * blade_steps_per_mm < max_step_hz);
// Driver requirements: >= 1 us high and low; PIO clock 1 MHz gives 1 us granularity.
inline constexpr std::uint32_t pio_clock_hz = 1000000;
inline constexpr std::uint32_t min_pulse_half_period_ticks = 1;
inline constexpr std::uint32_t dir_setup_us = 5;   // >= 2 us required by the drivers.
}

// ------------------------------------------------------- measuring encoders
namespace enc {
inline constexpr std::int32_t counts_per_rev = spec::encoder_counts_rev;              // 8192 (x4 decode)
inline constexpr double nominal_wheel_circumference_mm = spec::wheel_circumference_mm; // 200 mm ifm E60137 nominal, not calibration!
inline constexpr double nominal_um_per_count = nominal_wheel_circumference_mm * 1000.0 / counts_per_rev; // 24.414
// Calibration scale is stored as counts per metre in flash and set by the CLI
// after the docs/20 length record; nominal value until a physical calibration exists.
inline constexpr std::int32_t nominal_counts_per_metre = 40960;
inline constexpr std::int32_t max_slip_um = 1500;  // encoder vs commanded disagreement that trips a fault.
}

// ----------------------------------------------------- sensing/observability
// M01/M27 single active source of truth for what each physical sensor observes
// in every phase. X coordinates are machine millimetres from the cut plane X0,
// +X toward the piece/eject side (active coordinate contract). Sources:
// mechanism_spec.hpp meas_x=-205 and mechanism_cad.cpp second module X=-385
// (both stock side), feed_drive_spec.hpp near_nip_x=75.5 and max_centres=64.5
// (right-cassette nips at 75.5/140).
namespace sensing {
// Stock-side measuring wheels W0/W1: they ride continuous stock and can never
// observe a severed piece. Agreement between them is redundancy, not proof of
// absolute length (shared material/tread/temperature systematics).
inline constexpr double stock_wheel_far_x_mm = -385.0;
inline constexpr double stock_wheel_near_x_mm = -205.0;
// Downstream measuring station D (candidate A: a repackaged module inside the
// short-piece insulated window X38-140 from ENGINEERING_HANDOFF 3.1). Nominal
// station sits between the right-cassette nips 75.5/138-140. The first
// kinematic screen is NOT proof that the 63.66 mm wheel/encoder/housing fits:
// the mechanical fit at this station remains a Phase-E/M02 gate.
inline constexpr double piece_station_x_mm = 106.75;
// Discharge clearance sensor: through-beam/proximity across the takeaway path,
// observing bodies in the discharge zone. True (clear) while the path is empty.
inline constexpr double discharge_sensor_x_mm = 250.0;
inline constexpr double discharge_sensor_span_mm = 40.0;
// Waste-chute transit sensor: it watches the blade-region waste chute (a fall
// path below the wire lane, NOT the wire lane itself), so normal wire motion
// never presents. A released prime stub or insulation slug must produce a
// present->clear transit through the chute during the sweep window before the
// next feed or blade close can trap it.
inline constexpr double waste_chute_sensor_drop_mm = 30.0;
inline constexpr double waste_chute_sensor_span_mm = 30.0;
// Occupancy rules (physical, derived from material bodies - never from
// commanded motor steps): a body [tail, tail+len] occupies station s while
// tail <= s <= tail+len; the stock occupies s while stock_tip >= s. A station
// is invalid before the wire arrives and after the body leaves it; counts
// outside occupancy are physically zero and must never be fabricated or
// required. Every tracked move declares its observing station and expected
// observed distance; the unobserved remainder must be covered by discharge
// evidence (final ejection only) or the move is a design error.
inline constexpr double piece_body_grip_x_mm = 140.0; // far right nip: below
// this length a severed body cannot be propelled by the right cassette and is
// handled as a loose stub by the waste sweep (10 mm prime stub case).
}

// Legacy-constant quarantine (M01): machine_spec.hpp still carries historical
// feed_steps_mm (3200 pulses/rev family) and max_step_hz = 40000. The single
// active production source of truth is this header (2000 pulses/rev, 8000 Hz).
// The legacy values must never drive a production consumer.
static_assert(axis::feed_pulses_per_rev == 2000.0, "active feed scale drifted");
static_assert(axis::max_step_hz == 8000, "active step-rate ceiling drifted");
static_assert(spec::legacy_feed_pulses_per_rev != axis::feed_pulses_per_rev,
              "legacy and active feed scales must stay distinguishable");

// ------------------------------------------------------------- job envelope
namespace job {
inline constexpr std::int32_t min_length_um = spec::min_length_um;   // 200000
inline constexpr std::int32_t max_length_um = spec::max_length_um;   // 3000000
inline constexpr std::int32_t min_strip_um  = spec::min_strip_um;    // 3000
inline constexpr std::int32_t max_strip_um  = spec::max_strip_um;    // 25000
inline constexpr std::int32_t min_strip_depth_um = 100;
inline constexpr std::int32_t max_strip_depth_um = static_cast<std::int32_t>(axis::blade_cut_mm * 1000) - 100; // 4000
inline constexpr std::uint32_t max_quantity = spec::max_quantity;    // 1000000
inline constexpr std::int32_t wire_min_um = static_cast<std::int32_t>(feed::design::wire_diameter_min * 1000); // 1200
inline constexpr std::int32_t wire_max_um = static_cast<std::int32_t>(feed::design::wire_diameter_max * 1000); // 4500
inline constexpr std::int32_t ejection_um = spec::ejection_um;       // 230000
}

// --------------------------------------------------------------- supervision
namespace timing {
inline constexpr std::uint32_t input_debounce_ms = 20;
// Both selected V4.1 manuals state a 200 ms minimum and depict t1 >300 ms.
// GPIO is published on the following 1 kHz tick, so 350 ms guarantees >300 ms
// from the physical ENA transition to the first possible PUL edge.
inline constexpr std::uint32_t drive_enable_settle_ms = 350;
inline constexpr std::uint32_t reset_hold_ms = 500;         // RESET must be held to acknowledge faults.
inline constexpr std::uint32_t start_hold_ms = 100;
inline constexpr std::uint32_t homing_timeout_ms = 20000;
inline constexpr double homing_search_mm = 30.0;     // maximum travel toward open before the sensor must be seen.
inline constexpr double homing_fast_mm_s = 2.0, homing_slow_mm_s = 0.25, homing_backoff_mm = 1.0;
inline constexpr std::uint32_t move_timeout_ms = static_cast<std::uint32_t>(spec::move_timeout_us / 1000); // 30000
inline constexpr std::uint32_t host_link_timeout_ms = 750;  // only enforced while a host session is open.
inline constexpr std::uint32_t buzzer_fault_ms = 2000;
inline constexpr std::uint32_t blink_slow_ms = 500, blink_fast_ms = 125;
// Handling verification windows (M27/M30). The discharge present->clear pair is
// validated over the physical observation window that starts at ejection, so a
// part that clears before PieceDone is still evidenced (and one that never shows
// present or never clears faults and is quarantined).
inline constexpr std::uint32_t discharge_clear_timeout_ms = 5000;
inline constexpr std::uint32_t discharge_window_pre_ms = 100;  // slack before ejection start
// Waste sweep: constrained mechanical sweep actuation, then a required
// present->clear transit on the waste-chute sensor before the next feed or
// blade close may trap a released stub/slug.
inline constexpr std::uint32_t waste_sweep_ms = 200;
inline constexpr std::uint32_t waste_clear_timeout_ms = 3000;
}
}
