#pragma once
// Trapezoidal step scheduling, computed lazily pulse by pulse so a controlled
// stop can be inserted at any moment. Host-tested; used unchanged on RP2350.
#include <cstdint>
#include "types.hpp"

namespace starz::fw {

// Wire period of one pulse = (2 * y + pio_period_overhead_ticks) PIO ticks,
// where y is StepRun::half_period_ticks. Fixed by rp2350/step_gen.pio (five
// instructions between pulses on both the fetch and continuation paths).
inline constexpr std::uint32_t pio_period_overhead_ticks = 9;
inline constexpr std::uint32_t min_half_period_ticks = 1;
// Longest run handed to the PIO FIFO; bounds the latency of a stop request.
inline constexpr std::uint32_t max_run_pulses = 8;

struct MoveSpec {
    std::uint32_t steps = 0;            // total pulses, > 0
    double steps_per_s = 0;             // cruise limit
    double steps_per_s2 = 0;            // acceleration magnitude
    bool forward = true;
};

class StepStreamer {
public:
    // Returns false if the specification is invalid (zero steps/speed/accel).
    // max_run limits pulses per StepRun (1 during homing approach so that an
    // abort on the sensor edge overshoots by at most one pulse).
    bool begin(const MoveSpec& spec, std::uint32_t tick_hz, std::uint32_t max_run = max_run_pulses) noexcept;

    // Request a controlled deceleration from the current speed. Idempotent.
    void begin_stop() noexcept;

    // Pop the next run for the PIO. Returns false when the move is finished.
    bool pop(StepRun& run) noexcept;

    bool active() const noexcept { return active_; }
    bool stopping() const noexcept { return stopping_; }
    std::uint32_t issued() const noexcept { return issued_; }
    std::uint32_t planned() const noexcept { return spec_.steps; }
    bool forward() const noexcept { return spec_.forward; }

    // Elapsed PIO ticks covered by all runs popped so far.
    std::uint64_t issued_ticks() const noexcept { return emitted_ticks_; }

private:
    double time_of_step(std::uint32_t i) const noexcept; // seconds at which pulse i starts
    double ideal_dt(std::uint32_t i, std::uint32_t stop_k) const noexcept;
    std::uint32_t choose_half_period(double dt, std::uint64_t target, std::uint64_t emitted) const noexcept;
    void commit(double dt, std::uint32_t y) noexcept;
    std::uint32_t remaining() const noexcept;

    MoveSpec spec_{};
    std::uint32_t tick_hz_ = 0;
    std::uint32_t max_run_ = max_run_pulses;
    std::uint32_t n_acc_ = 0, n_dec_ = 0; // ramp lengths in steps
    double v_peak_ = 0;                  // steps/s reached
    double t_acc_end_ = 0, t_cruise_end_ = 0;
    std::uint32_t issued_ = 0;
    std::uint64_t target_ticks_ = 0;     // ideal cumulative ticks at pulse `issued_`
    std::uint64_t emitted_ticks_ = 0;    // actual cumulative ticks emitted
    bool active_ = false;
    bool stopping_ = false;
    double stop_v0_ = 0;                 // speed when stop began
    std::uint32_t stop_steps_ = 0, stop_issued_ = 0;
};

} // namespace starz::fw
