#pragma once
// Firmware core data types. No Pico SDK dependency: this header is compiled and
// tested on the host and cross-compiled unchanged for the RP2350.
#include <cstdint>
#include "controls_spec.hpp"

namespace starz::fw {

using Axis = ctrl::axis::Id;

enum class State : std::uint8_t {
    PowerOn = 0,   // outputs safe, nothing known.
    Fault = 1,     // latched; RESET held or CLI RESET with healthy inputs clears.
    Unhomed = 2,   // healthy, blades not referenced.
    Homing = 3,
    Ready = 4,     // homed, blades open, idle.
    Running = 5,   // executing job cycles.
    Stopping = 6,  // finish current axis moves at controlled decel, then Ready/Paused.
    Paused = 7,    // job retained, START resumes at the cycle boundary.
    Jogging = 8,
};

// Fault bits: sticky until acknowledged. Bits 0-3 are the driver alarms.
namespace fault {
inline constexpr std::uint16_t alm_blade_front = 1u << 0;
inline constexpr std::uint16_t alm_blade_rear  = 1u << 1;
inline constexpr std::uint16_t alm_feed_left   = 1u << 2;
inline constexpr std::uint16_t alm_feed_right  = 1u << 3;
inline constexpr std::uint16_t safety_open     = 1u << 4;  // XPSUAF13AP enabling contact opened (E-stop/guard).
inline constexpr std::uint16_t stop_pressed    = 1u << 5;  // STOP NC loop open while moving.
inline constexpr std::uint16_t homing_failed   = 1u << 6;
inline constexpr std::uint16_t move_timeout    = 1u << 7;
inline constexpr std::uint16_t length_slip     = 1u << 8;  // measuring encoder vs command beyond enc::max_slip_um.
inline constexpr std::uint16_t no_wire         = 1u << 9;  // job started with the measuring encoder not seeing wire motion.
inline constexpr std::uint16_t blade_limit     = 1u << 10; // commanded blade travel beyond axis::blade_limit_mm.
inline constexpr std::uint16_t encoder_stalled = 1u << 11; // feed commanded, encoder did not move.
inline constexpr std::uint16_t encoder_overflow = 1u << 12; // PIO encoder event FIFO overflowed; position is incomplete.
inline constexpr std::uint16_t watchdog_reset  = 1u << 13; // board reset because the watchdog was not serviced.
inline constexpr std::uint16_t encoder_invalid  = 1u << 14; // non-Gray encoder transition; direction/count cannot be trusted.
inline constexpr std::uint16_t internal        = 1u << 15; // invariant violated in firmware.
}

struct Job {
    std::int32_t length_um = 0;        // finished piece length.
    std::int32_t strip_lead_um = 0;    // 0 = no lead strip.
    std::int32_t strip_tail_um = 0;    // 0 = no tail strip.
    std::int32_t strip_depth_um = 0;   // radial insulation depth removed per side.
    std::int32_t wire_diameter_um = 0; // outside diameter over insulation.
    std::uint32_t quantity = 0;
};

// Persistent settings, stored by Board::store_settings. A valid saved recipe
// is restored after a power cycle, so the panel START button is sufficient for
// normal screenless operation once the recipe has been commissioned over USB.
struct Settings {
    static constexpr std::uint32_t stored_job_marker = 0x53544A31u; // "STJ1"

    std::uint32_t magic = 0x53545a31; // "STZ1"
    std::int32_t counts_per_metre[2] = {ctrl::enc::nominal_counts_per_metre, ctrl::enc::nominal_counts_per_metre};
    std::uint32_t pieces_total = 0;   // lifetime counter
    Job stored_job{};
    std::uint32_t stored_job_valid = 0;
    std::uint32_t crc = 0;            // CRC-16/CCITT-FALSE of the preceding bytes, zero-extended
};

static_assert(sizeof(Job) == 24, "Job layout changed; review protocol and persistent-settings encoding");
static_assert(sizeof(Settings) == 48, "Settings layout changed; review RP2350 flash record capacity and CRC coverage");

enum class JobError : std::uint8_t {
    None = 0, Length, StripLead, StripTail, Depth, WireDiameter, Quantity, Geometry
};

// Field-level validation against shared/controls_spec.hpp. No material or
// process suitability is inferred from a passing job.
JobError validate(const Job& j) noexcept;

struct Inputs {
    bool start = false;      // START pressed.
    bool stop_ok = false;    // STOP loop closed (button not pressed).
    bool reset = false;      // RESET pressed.
    bool safety_ok = false;  // safety relay enabling contact closed.
    bool home_front = false; // blade front at open-end sensor.
    bool home_rear = false;
    bool alm[4] = {false, false, false, false}; // indexed by Axis.
};

struct Outputs {
    bool led_green = false, led_red = false, buzzer = false;
    bool drives_disabled = true;
};

// One PIO run-length entry: `count` pulses, each with `half_period_ticks` high
// and low at ctrl::axis::pio_clock_hz. The PIO program adds a fixed overhead
// (see step_gen.pio); the planner compensates so the wire period is exact.
struct StepRun {
    std::uint32_t count;
    std::uint32_t half_period_ticks;
};

} // namespace starz::fw
