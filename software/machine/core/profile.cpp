#include "profile.hpp"
#include <cmath>
#include <algorithm>

namespace starz::fw {

bool StepStreamer::begin(const MoveSpec& spec, std::uint32_t tick_hz, std::uint32_t max_run) noexcept {
    if (spec.steps == 0 || !(spec.steps_per_s > 0) || !(spec.steps_per_s2 > 0) || tick_hz == 0 || max_run == 0) return false;
    spec_ = spec;
    tick_hz_ = tick_hz;
    max_run_ = max_run > max_run_pulses ? max_run_pulses : max_run;
    issued_ = 0;
    target_ticks_ = 0;
    emitted_ticks_ = 0;
    stopping_ = false;
    stop_issued_ = 0;
    stop_steps_ = 0;

    const double n = static_cast<double>(spec.steps);
    const double n_ramp_full = spec.steps_per_s * spec.steps_per_s / (2.0 * spec.steps_per_s2);
    if (2.0 * n_ramp_full <= n) {
        n_acc_ = std::max<std::uint32_t>(1, static_cast<std::uint32_t>(std::floor(n_ramp_full)));
        n_dec_ = spec.steps >= 2 ? n_acc_ : 0;
        v_peak_ = spec.steps_per_s;
    } else {
        n_acc_ = spec.steps / 2;
        n_dec_ = spec.steps - n_acc_;
        if (n_acc_ == 0) { n_acc_ = 1; n_dec_ = 0; }
        v_peak_ = std::min(std::sqrt(2.0 * spec.steps_per_s2 * static_cast<double>(n_acc_)), spec.steps_per_s);
    }
    t_acc_end_ = std::sqrt(2.0 * static_cast<double>(n_acc_) / spec.steps_per_s2);
    // v_peak_ is the cruise speed; the ramp ends at sqrt(2 a n_acc) <= v_peak_,
    // within one acceleration increment of it.
    const double cruise_steps = n - static_cast<double>(n_acc_) - static_cast<double>(n_dec_);
    t_cruise_end_ = t_acc_end_ + (cruise_steps > 0 ? cruise_steps / v_peak_ : 0.0);
    active_ = true;
    return true;
}

double StepStreamer::time_of_step(std::uint32_t i) const noexcept {
    const double a = spec_.steps_per_s2;
    const double s = static_cast<double>(std::min(i, spec_.steps));
    if (i <= n_acc_) return std::sqrt(2.0 * s / a);
    const double cruise_steps = static_cast<double>(spec_.steps - n_acc_ - n_dec_);
    if (s <= static_cast<double>(n_acc_) + cruise_steps) return t_acc_end_ + (s - static_cast<double>(n_acc_)) / v_peak_;
    const double remaining_steps = static_cast<double>(spec_.steps) - s;
    const double t_total = t_cruise_end_ + std::sqrt(2.0 * static_cast<double>(n_dec_) / a);
    return t_total - std::sqrt(2.0 * remaining_steps / a);
}

void StepStreamer::begin_stop() noexcept {
    if (!active_ || stopping_) return;
    stopping_ = true;
    const std::uint32_t i = std::min(issued_, spec_.steps - 1);
    const double dt = std::max(time_of_step(i + 1) - time_of_step(i), 1.0 / static_cast<double>(tick_hz_));
    stop_v0_ = std::min(1.0 / dt, v_peak_);
    stop_steps_ = static_cast<std::uint32_t>(std::ceil(stop_v0_ * stop_v0_ / (2.0 * spec_.steps_per_s2)));
    stop_steps_ = std::clamp<std::uint32_t>(stop_steps_, 1, spec_.steps - issued_);
    stop_issued_ = 0;
}

double StepStreamer::ideal_dt(std::uint32_t i, std::uint32_t stop_k) const noexcept {
    if (stopping_) {
        const double a = spec_.steps_per_s2;
        const double v2 = std::max(stop_v0_ * stop_v0_ - 2.0 * a * static_cast<double>(stop_k), a);
        return 1.0 / std::sqrt(v2);
    }
    return time_of_step(i + 1) - time_of_step(i);
}

std::uint32_t StepStreamer::choose_half_period(double dt, std::uint64_t target, std::uint64_t emitted) const noexcept {
    const std::uint64_t new_target = target + static_cast<std::uint64_t>(std::llround(dt * static_cast<double>(tick_hz_)));
    const std::int64_t want = static_cast<std::int64_t>(new_target) - static_cast<std::int64_t>(emitted);
    std::int64_t y = (want - static_cast<std::int64_t>(pio_period_overhead_ticks)) / 2;
    if (y < static_cast<std::int64_t>(min_half_period_ticks)) y = min_half_period_ticks;
    return static_cast<std::uint32_t>(y);
}

void StepStreamer::commit(double dt, std::uint32_t y) noexcept {
    target_ticks_ += static_cast<std::uint64_t>(std::llround(dt * static_cast<double>(tick_hz_)));
    emitted_ticks_ += 2ull * y + pio_period_overhead_ticks;
    ++issued_;
    if (stopping_) ++stop_issued_;
}

std::uint32_t StepStreamer::remaining() const noexcept {
    return stopping_ ? (stop_steps_ - stop_issued_) : (spec_.steps - issued_);
}

bool StepStreamer::pop(StepRun& run) noexcept {
    if (!active_) return false;
    if (remaining() == 0) { active_ = false; return false; }
    const double dt0 = ideal_dt(issued_, stop_issued_);
    const std::uint32_t y0 = choose_half_period(dt0, target_ticks_, emitted_ticks_);
    commit(dt0, y0);
    std::uint32_t count = 1;
    while (count < max_run_ && remaining() > 0) {
        const double dt = ideal_dt(issued_, stop_issued_);
        const std::uint32_t y = choose_half_period(dt, target_ticks_, emitted_ticks_);
        if (y != y0) break;
        commit(dt, y);
        ++count;
    }
    run.count = count;
    run.half_period_ticks = y0;
    if (remaining() == 0) active_ = false;
    return true;
}

} // namespace starz::fw
