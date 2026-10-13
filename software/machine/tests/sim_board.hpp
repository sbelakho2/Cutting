#pragma once
// Simulated Board for host tests: models the PIO step FIFOs, blade home
// sensors, feed roller encoders (with optional slip) and the field inputs.
#include "board.hpp"
#include "profile.hpp"
#include "controls_spec.hpp"
#include <deque>
#include <cmath>
#include <cstring>
#include <vector>

namespace starz::fw::sim {

class SimBoard final : public Board {
public:
    struct Pending { std::uint32_t remaining; std::uint32_t half_period; std::uint32_t phase_ticks; bool forward; };

    // ----- Board
    std::uint64_t now_us() override { return now_us_; }
    Inputs read_inputs() override {
        Inputs i = field;
        i.home_front = blade_on_sensor(0);
        i.home_rear = blade_on_sensor(1);
        return i;
    }
    void write_outputs(const Outputs& o) override { outputs = o; }
    void set_direction(Axis a, bool forward) override { dir_[a] = forward; }
    std::uint32_t step_fifo_level(Axis a) override { return static_cast<std::uint32_t>(fifo_[a].size()); }
    void push_steps(Axis a, const StepRun& run) override {
        if (fifo_[a].size() >= step_fifo_depth) return;
        fifo_[a].push_back(Pending{run.count, run.half_period_ticks, 0, dir_[a]});
    }
    bool steps_idle(Axis a) override { return fifo_[a].empty(); }
    void abort_steps(Axis a) override { fifo_[a].clear(); }
    std::int64_t encoder_count(std::uint8_t side) override { return encoder_[side]; }
    bool encoder_healthy(std::uint8_t side) override { return encoder_ok[side]; }
    FaultBits boot_faults() override { return boot_fault_bits; }
    FaultBits latched_faults() override { return runtime_fault_bits; }
    bool clear_latched_faults() override {
        if (!clear_latches_allowed) return false;
        runtime_fault_bits = 0;
        encoder_ok[0] = true;
        encoder_ok[1] = true;
        return true;
    }
    bool load_settings(void* dst, std::uint32_t n) override {
        if (flash.size() != n) return false;
        std::memcpy(dst, flash.data(), n); return true;
    }
    bool store_settings(const void* src, std::uint32_t n) override {
        if (!store_settings_allowed) return false;
        flash.assign(static_cast<const std::uint8_t*>(src), static_cast<const std::uint8_t*>(src) + n);
        ++stores; return true;
    }
    void watchdog_feed() override { ++feeds; }

    // ----- simulation control
    // Advance simulated time by us, stepping each axis's FIFO at the PIO clock.
    void advance(std::uint32_t us) {
        for (std::uint32_t t = 0; t < us; ++t) {
            ++now_us_;
            for (std::uint8_t ax = 0; ax < ctrl::axis::count; ++ax) step_axis(static_cast<Axis>(ax));
        }
    }
    bool blade_on_sensor(int i) const {
        // IE5338 at the open end: active while blade travel <= sensor_window_um
        // (travel measured from the true open position; the simulation starts
        // the blades at start_offset_um).
        return blade_true_um[i] <= sensor_window_um;
    }

    // mechanical state (true positions)
    double blade_true_um[2] = {5000.0, 5000.0};   // starts away from the sensor
    double feed_true_um[2] = {0.0, 0.0};
    double sensor_window_um = 200.0;
    double slip_ppm[2] = {0.0, 0.0};                // encoder relative error (positive = encoder reads long)
    // Encoder wiring polarity and stuck/frozen feedback (commissioning tests).
    int encoder_sign[2] = {1, 1};
    bool encoder_freeze[2] = {false, false};
    bool wire_present = true;
    bool encoder_ok[2] = {true, true};
    FaultBits boot_fault_bits = 0;
    FaultBits runtime_fault_bits = 0;
    bool clear_latches_allowed = true;
    std::int64_t encoder_[2] = {0, 0};
    double enc_accum_[2] = {0.0, 0.0};
    std::uint32_t step_period_hist[64]{};           // histogram of wire periods (ticks), capped
    std::uint64_t pulses[ctrl::axis::count]{};
    std::uint32_t min_half_period_seen = ~0u;
    Inputs field{};
    Outputs outputs{};
    std::vector<std::uint8_t> flash;
    int stores = 0;
    bool store_settings_allowed = true;
    std::uint64_t feeds = 0;

    SimBoard() {
        field = Inputs{};
        field.start = false; field.stop_ok = true; field.reset = false; field.safety_ok = true; field.part_clear = true;
    }

private:
    void step_axis(Axis a) {
        if (fifo_[a].empty()) return;
        Pending& p = fifo_[a].front();
        // Wire period = 2*half_period + pio_period_overhead_ticks (one tick per us at 1 MHz).
        const std::uint32_t period = 2 * p.half_period + pio_period_overhead_ticks;
        if (++p.phase_ticks >= period) {
            p.phase_ticks = 0;
            apply_step(a, p.forward);
            if (p.half_period < min_half_period_seen) min_half_period_seen = p.half_period;
            if (period < 64) ++step_period_hist[period];
            if (--p.remaining == 0) fifo_[a].pop_front();
        }
    }
    void apply_step(Axis a, bool forward) {
        ++pulses[a];
        const double s = forward ? 1.0 : -1.0;
        if (a == ctrl::axis::blade_front || a == ctrl::axis::blade_rear) {
            blade_true_um[a] += s * 1000.0 / ctrl::axis::blade_steps_per_mm;
        } else {
            const int side = a == ctrl::axis::feed_right ? 1 : 0;
            const double d = s * 1000.0 / ctrl::axis::feed_steps_per_mm;
            feed_true_um[side] += d;
            if (!wire_present || encoder_freeze[side]) return;
            enc_accum_[side] += d * encoder_sign[side] * (1.0 + slip_ppm[side] * 1e-6) * ctrl::enc::nominal_counts_per_metre / 1e6;
            const double whole = std::trunc(enc_accum_[side]);
            encoder_[side] += static_cast<std::int64_t>(whole);
            enc_accum_[side] -= whole;
        }
    }
    std::uint64_t now_us_ = 0;
    bool dir_[ctrl::axis::count]{};
    std::deque<Pending> fifo_[ctrl::axis::count];
};

} // namespace starz::fw::sim
