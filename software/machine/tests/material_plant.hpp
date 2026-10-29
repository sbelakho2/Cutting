#pragma once
// Independent material plant for host tests (M27/M30). Unlike the legacy
// SimBoard - whose apply_step() advances each encoder from its motor's
// commanded steps (the defect demonstrated by the preserved stock-side
// counterexample) - this plant models actual material bodies and generates
// sensor counts ONLY from material motion at physically occupied stations:
//
//   stock body:  semi-infinite wire from the payoff, leading end stock_tip_um.
//   piece body:  severed part [piece_tail_um, +piece_len_um], grippable by the
//                right cassette only while it spans the far right nip
//                (ctrl::sensing::piece_body_grip_x_mm = 140 mm). A shorter
//                severed body (the 10 mm prime stub) is never propelled by feed
//                motion and becomes a blade-pocket waste body.
//   waste bodies: released stubs/slugs in the blade pocket, swept through the
//                waste chute (a fall path off the wire lane) by Outputs::
//                waste_sweep; the chute sensor sees only the falling transit.
//
// Stations (ctrl::sensing): W0/W1 stock wheels ride continuous stock and can
// never observe a severed piece; station D observes whichever body occupies
// it (stock before severing while the stock tip has passed it, piece after
// severing while the piece spans it) and accumulates nothing outside
// occupancy. The discharge sensor watches the lane zone [230,270] mm.
//
// The plant receives only actuator commands and its own physics; it never
// reads controller or job state. Fault injections (slip, frozen stations,
// wire break, stuck sensors, blocked sweep, gripper spin) are explicit
// physical disturbances.
#include "board.hpp"
#include "profile.hpp"
#include "controls_spec.hpp"
#include <cmath>
#include <cstring>
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

namespace starz::fw::sim {

class MaterialPlant final : public Board {
public:
    struct Pending { std::uint32_t remaining; std::uint32_t half_period; std::uint32_t phase_ticks; bool forward; };

    // ----- Board
    std::uint64_t now_us() override { return now_us_; }
    Inputs read_inputs() override {
        Inputs i = field;
        i.home_front = blade_true_um[0] <= sensor_window_um;
        i.home_rear = blade_true_um[1] <= sensor_window_um;
        i.part_clear = part_clear_stuck.has_value() ? *part_clear_stuck : !discharge_body_present();
        i.waste_clear = waste_clear_stuck.has_value() ? *waste_clear_stuck : !waste_sensor_present();
        return i;
    }
    void write_outputs(const Outputs& o) override {
        if (o.waste_sweep && !outputs.waste_sweep) sweep_on_us_ = now_us_;
        outputs = o;
    }
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
    std::int64_t piece_encoder_count() override { return encoder_[2]; }
    bool piece_encoder_healthy() override { return encoder_ok[2]; }
    FaultBits boot_faults() override { return boot_fault_bits; }
    FaultBits latched_faults() override { return runtime_fault_bits; }
    bool clear_latched_faults() override {
        if (!clear_latches_allowed) return false;
        runtime_fault_bits = 0;
        for (bool& ok : encoder_ok) ok = true;
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
    void advance(std::uint32_t us) {
        for (std::uint32_t t = 0; t < us; ++t) {
            ++now_us_;
            for (std::uint8_t ax = 0; ax < ctrl::axis::count; ++ax) step_axis(static_cast<Axis>(ax));
            physics_tick();
        }
    }

    // ----- physical state (true positions, micrometres from the cut plane X0)
    double blade_true_um[2] = {5000.0, 5000.0};   // travel from open; starts away from the sensor
    double sensor_window_um = 200.0;
    double stock_tip_um = -3000.0;                // tip 3 mm behind the plane at boot
    double piece_tail_um = 0.0;
    double piece_len_um = 0.0;
    bool piece_present = false;
    bool piece_on_takeaway = false;               // past the far nip: coasting to the tray
    double feed_true_um[2] = {0.0, 0.0};          // commanded roller travel (not material truth)

    // Sensor-station physics: [0]=W0 (X=-385), [1]=W1 (X=-205), [2]=D (piece station).
    double slip_ppm[3] = {0.0, 0.0, 0.0};
    int encoder_sign[3] = {1, 1, 1};              // wiring polarity of each station
    bool encoder_freeze[3] = {false, false, false};
    bool encoder_ok[3] = {true, true, true};
    std::int64_t encoder_[3] = {0, 0, 0};
    double enc_accum_[3] = {0.0, 0.0, 0.0};
    double left_grip_slip_ppm = 0.0;              // stock transport error at the left cassette
    double right_grip_slip_ppm = 0.0;
    bool wire_present = true;                     // stock exists at all (wire-break injection)
    bool sweep_blocked = false;                   // released body refuses to leave the pocket

    // Sensor fault injections (physically stuck channels).
    std::optional<bool> part_clear_stuck;
    std::optional<bool> waste_clear_stuck;

    FaultBits boot_fault_bits = 0;
    FaultBits runtime_fault_bits = 0;
    bool clear_latches_allowed = true;
    std::uint32_t step_period_hist[64]{};
    std::uint64_t pulses[ctrl::axis::count]{};
    std::uint32_t min_half_period_seen = ~0u;
    Inputs field{};
    Outputs outputs{};
    std::vector<std::uint8_t> flash;
    int stores = 0;
    bool store_settings_allowed = true;
    std::uint64_t feeds = 0;

    MaterialPlant() {
        field = Inputs{};
        field.start = false; field.stop_ok = true; field.reset = false; field.safety_ok = true;
        // Unassigned/stuck-safe defaults: the discharge channel reads NOT clear
        // until a body actually presents and clears; the waste chute is empty.
        field.part_clear = false;
        field.waste_clear = true;
    }

    // ----- introspection for tests
    std::size_t pocket_waste_bodies() const { return pocket_.size(); }
    std::size_t waste_bin_count() const { return bin_count_; }
    bool score_pending() const { return score_armed_; }
    double score_separation() const { return score_separation_um_; }
    double score_end0() const { return score_end0_um_; }
    bool discharge_body_present() const {
        const double lo = ctrl::sensing::discharge_sensor_x_mm * 1000.0 -
                          ctrl::sensing::discharge_sensor_span_mm * 500.0;
        const double hi = ctrl::sensing::discharge_sensor_x_mm * 1000.0 +
                          ctrl::sensing::discharge_sensor_span_mm * 500.0;
        return piece_present && piece_tail_um < hi && piece_tail_um + piece_len_um > lo;
    }
    bool waste_sensor_present() const { return falling_ && fall_progress_ >= 0.2 && fall_progress_ <= 0.8; }

private:
    struct PocketBody { double x0, x1; }; // lane extent at release (audit only)
    std::deque<Pending> fifo_[ctrl::axis::count];
    std::vector<PocketBody> pocket_;      // released stubs/slugs at the blade pocket
    std::size_t bin_count_ = 0;           // bodies that reached the waste bin
    bool falling_ = false;
    double fall_progress_ = 0.0;
    std::uint64_t sweep_on_us_ = 0;

    // Blade close-event bookkeeping (sever/score detection from true travel).
    bool sever_latched_ = false;
    bool score_armed_ = false;
    bool score_body_is_piece_ = false;
    double score_end0_um_ = 0.0;
    double score_separation_um_ = 0.0;

    void step_axis(Axis a) {
        if (fifo_[a].empty()) return;
        Pending& p = fifo_[a].front();
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
            const int i = a == ctrl::axis::blade_front ? 0 : 1;
            blade_true_um[i] += s * 1000.0 / ctrl::axis::blade_steps_per_mm;
            detect_blade_event();
            return;
        }
        const int side = a == ctrl::axis::feed_right ? 1 : 0;
        const double d = s * 1000.0 / ctrl::axis::feed_steps_per_mm;
        feed_true_um[side] += d;
        if (side == 0) {
            last_left_step_us_ = now_us_;
            // Left cassette grips and moves the stock (stock master).
            if (!wire_present) return;
            const double motion = d * (1.0 + left_grip_slip_ppm * 1e-6);
            move_stock(motion);
        } else {
            // Right cassette grips the severed piece while it spans the far
            // right nip. With no piece it grips the stock only while the stock
            // reaches its nips - and drives it only when it is not stepping in
            // synchronised feed with the stock master (an equal simultaneous
            // command moves the shared body once, not twice). A loose stub is
            // never propelled, and past the far nip the rollers spin free.
            if (piece_present && !piece_on_takeaway && piece_covers_far_nip()) {
                const double motion = d * (1.0 + right_grip_slip_ppm * 1e-6);
                move_piece(motion);
                return;
            }
            const bool left_driving = (now_us_ - last_left_step_us_ <= 2000) ||
                                      !fifo_[ctrl::axis::feed_left].empty();
            const double near_nip = feed::design::near_nip_x * 1000.0;
            if (!left_driving && wire_present && stock_tip_um >= near_nip) {
                const double motion = d * (1.0 + right_grip_slip_ppm * 1e-6);
                move_stock(motion);
            }
        }
    }

    bool piece_covers_far_nip() const {
        const double far_nip = ctrl::sensing::piece_body_grip_x_mm * 1000.0;
        return piece_present && piece_tail_um <= far_nip && piece_tail_um + piece_len_um >= far_nip;
    }

    void move_stock(double motion) {
        stock_tip_um += motion;
        accumulate(0, motion);
        accumulate(1, motion);
        // Station D observes stock motion while the stock tip has passed it.
        const double d_station = ctrl::sensing::piece_station_x_mm * 1000.0;
        if (stock_tip_um >= d_station) accumulate(2, motion);
        if (score_armed_ && !score_body_is_piece_) score_separation_um_ += std::fabs(motion);
    }

    void move_piece(double motion) {
        piece_tail_um += motion;
        const double d_station = ctrl::sensing::piece_station_x_mm * 1000.0;
        if (piece_tail_um <= d_station && piece_tail_um + piece_len_um >= d_station) accumulate(2, motion);
        if (score_armed_ && score_body_is_piece_) score_separation_um_ += std::fabs(motion);
    }

    // One measuring station accumulates counts only from material motion while
    // physically occupied - never from commanded steps (M27).
    void accumulate(int station, double motion_um) {
        if (encoder_freeze[station]) return;
        enc_accum_[station] += motion_um * encoder_sign[station] * (1.0 + slip_ppm[station] * 1e-6) *
                                ctrl::enc::nominal_counts_per_metre / 1e6;
        const double whole = std::trunc(enc_accum_[station]);
        encoder_[station] += static_cast<std::int64_t>(whole);
        enc_accum_[station] -= whole;
    }

    void detect_blade_event() {
        // Sever is one physical event: latch it when either jaw reaches sever
        // travel and release only when both jaws have fully reopened (a shared
        // latch cleared per-jaw would re-fire the split).
        if (!sever_latched_ && (blade_true_um[0] >= 4050.0 || blade_true_um[1] >= 4050.0)) {
            sever_latched_ = true;
            on_sever();
        } else if (sever_latched_ && blade_true_um[0] < 3900.0 && blade_true_um[1] < 3900.0) {
            sever_latched_ = false;
        }
    }

    void detect_score_event() {
        // A score is a settled blade close that stops short of sever travel
        // with material spanning the plane. Settle detection (not a crossing)
        // so a sever move passing through the band is never misread as a score.
        if (score_armed_) return;
        if (!(fifo_[ctrl::axis::blade_front].empty() && fifo_[ctrl::axis::blade_rear].empty())) return;
        const double travel = std::fmax(blade_true_um[0], blade_true_um[1]);
        if (!(travel >= 1500.0 && travel < 4000.0)) return;
        const bool stock_at_plane = wire_present && stock_tip_um > 0.0;
        const bool piece_at_plane = piece_present && piece_tail_um <= 0.0 && piece_tail_um + piece_len_um > 0.0;
        if (!stock_at_plane && !piece_at_plane) return;
        score_armed_ = true;
        score_separation_um_ = 0.0;
        if (piece_at_plane) {
            score_body_is_piece_ = true;
            score_end0_um_ = piece_tail_um;
        } else {
            score_body_is_piece_ = false;
            score_end0_um_ = stock_tip_um;
        }
    }

    void on_sever() {
        // Blade severs at the plane X0: the stock ends at 0 and the segment
        // [0, old_tip] becomes an independent body. A body that cannot reach
        // the far right nip is a loose stub (prime waste), not a piece.
        const double old_tip = stock_tip_um;
        if (!(old_tip > 0.0) || !wire_present) return;
        stock_tip_um = 0.0;
        if (old_tip >= ctrl::sensing::piece_body_grip_x_mm * 1000.0) {
            piece_present = true;
            piece_on_takeaway = false;
            piece_tail_um = 0.0;
            piece_len_um = old_tip;
        } else {
            pocket_.push_back(PocketBody{0.0, old_tip}); // released at blade open; see physics_tick
        }
    }

    void on_score() = delete; // score detection is detect_score_event() at settle

    void physics_tick() {
        detect_score_event();
        // Slug release: after a scored body separates past the bare-end
        // clearance while the blades hold, the slug is a pocket waste body.
        // "Jaws open" means travel back inside the open window; the homing
        // reference lands within the 200 um sensor window, so the release
        // threshold must clear that offset.
        if (score_armed_ && blade_true_um[0] <= 500.0 && blade_true_um[1] <= 500.0) {
            if (score_separation_um_ >= std::fabs(score_end0_um_) + 3000.0) {
                const double lo = score_end0_um_ < 0.0 ? score_end0_um_ : 0.0;
                const double hi = score_end0_um_ < 0.0 ? 0.0 : score_end0_um_;
                pocket_.push_back(PocketBody{lo, hi});
            }
            score_armed_ = false;
        }
        // Waste sweep: a pulsed sweep drops the pocket bodies through the
        // chute; the sensor sees only the falling transit. A blocked sweep
        // leaves the body in the pocket (no transit -> the controller faults).
        if (outputs.waste_sweep && !pocket_.empty() && !falling_ && !sweep_blocked &&
            now_us_ - sweep_on_us_ >= ctrl::timing::waste_sweep_ms * 500ull) {
            pocket_.erase(pocket_.begin());
            falling_ = true;
            fall_progress_ = 0.0;
        }
        if (falling_) {
            fall_progress_ += 1.0 / 100000.0; // 100 ms fall: robust against 20 ms input debounce
            if (fall_progress_ >= 1.0) { falling_ = false; fall_progress_ = 0.0; ++bin_count_; }
        }
        // Takeaway: once past the far nip the piece is no longer driven; the
        // designed output takeaway (sloped guided tray, validated in M30)
        // carries it out of the discharge zone.
        if (piece_present && piece_on_takeaway) {
            const double slide = 500000.0 / 1e6; // 500 mm/s, per-us increment
            piece_tail_um += slide;
            if (piece_tail_um >= 320000.0) {
                piece_present = false;
                piece_on_takeaway = false;
            }
        } else if (piece_present && piece_tail_um > ctrl::sensing::piece_body_grip_x_mm * 1000.0) {
            piece_on_takeaway = true;
        }
    }

    std::uint64_t now_us_ = 0;
    std::uint64_t last_left_step_us_ = ~0ull;
    bool dir_[ctrl::axis::count]{};
};

} // namespace starz::fw::sim
