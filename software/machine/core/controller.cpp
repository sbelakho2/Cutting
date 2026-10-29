#include "controller.hpp"
#include "crc16.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace starz::fw {

namespace {
constexpr std::uint64_t ms_to_us(std::uint32_t ms) { return static_cast<std::uint64_t>(ms) * 1000ull; }

std::int64_t um_to_feed_steps(std::int64_t um) {
    return std::llround(static_cast<double>(um) * ctrl::axis::feed_steps_per_mm / 1000.0);
}
std::int64_t um_to_blade_steps(std::int64_t um) {
    return std::llround(static_cast<double>(um) * ctrl::axis::blade_steps_per_mm / 1000.0);
}
std::int32_t blade_steps_to_um(std::int64_t steps) {
    return static_cast<std::int32_t>(std::llround(static_cast<double>(steps) * 1000.0 / ctrl::axis::blade_steps_per_mm));
}
std::int64_t feed_steps_to_um(std::int64_t steps) {
    return std::llround(static_cast<double>(steps) * 1000.0 / ctrl::axis::feed_steps_per_mm);
}
constexpr FaultBits alm_bit(Axis a) { return static_cast<FaultBits>(1u << static_cast<unsigned>(a)); }

// ---- process geometry (docs/01 §4, F08) -------------------------------------
// Ideal opposed-90-degree-V per-jaw closure to the incision circle of radius r:
//   closure = blade_open_apex/2 - sqrt(2)*r        (4.0 mm at an 8 mm open apex)
// The sever position stays at the CAD blade_cut_travel, which is the closed-jaw
// position plus an explicit per-jaw overlap (0.2 mm total apex overlap).
constexpr double blade_open_half_mm = ctrl::axis::blade_open_mm / 2.0; // 4.0
static_assert(ctrl::axis::blade_cut_mm > blade_open_half_mm, "sever travel must pass the closed-jaw position");
static_assert(ctrl::axis::blade_open_mm == head::design::blade_open_apex,
              "axis::blade_open_mm must stay equal to the CAD blade_open_apex");
static_assert(ctrl::axis::blade_cut_mm == head::design::blade_cut_travel,
              "axis::blade_cut_mm must stay equal to the CAD blade_cut_travel");
constexpr std::int32_t blade_sever_travel_um = static_cast<std::int32_t>(ctrl::axis::blade_cut_mm * 1000.0); // 4100
static_assert(blade_sever_overlap_um == blade_sever_travel_um - static_cast<std::int32_t>(blade_open_half_mm * 1000.0),
              "sever overlap must be the explicit per-jaw allowance");
static_assert(blade_sever_overlap_um == spec::blade_cut_position_um - spec::blade_open_apex_um / 2,
              "sever overlap must match the shared blade_cut_position_um constant");
static_assert(blade_sever_travel_um <= static_cast<std::int32_t>(ctrl::axis::blade_limit_mm * 1000.0),
              "sever travel must stay inside the software blade limit");

// ---- cycle geometry (docs/01 §4, F07) ---------------------------------------
constexpr std::int32_t strip_clear_um = spec::strip_clear_um;   // 3 mm lead/tail clearance at the bare end
static_assert(wire::separation_um == 10000, "tail-score separation is fixed at 10 mm by docs/01 §4");

// Commissioning polarity move (F33).
constexpr std::uint32_t commission_feed_move_um = 5000;
constexpr std::uint32_t commission_blade_move_um = 1000;
constexpr std::int64_t commission_min_counts = 16; // ~0.4 mm of measuring-wheel feedback

bool signs_valid(const Settings& s) noexcept {
    for (std::int8_t v : s.axis_sign) if (v != 1 && v != -1) return false;
    for (std::int8_t v : s.wheel_sign) if (v != 1 && v != -1) return false;
    return true;
}

// Deliberately excludes the final CRC word. The Settings layout assertion
// guarantees this covers the exact persisted payload.
std::uint32_t settings_crc(const Settings& s) {
    return crc16(reinterpret_cast<const std::uint8_t*>(&s), sizeof(Settings) - sizeof(std::uint32_t));
}
} // namespace

std::uint32_t commissioning_identity(const Settings& s, const char* firmware) noexcept {
    const char* fw = firmware != nullptr ? firmware : "";
    std::uint16_t hi = crc16(reinterpret_cast<const std::uint8_t*>(&s.counts_per_metre), sizeof s.counts_per_metre);
    hi = crc16(reinterpret_cast<const std::uint8_t*>(&s.stored_job), sizeof s.stored_job, hi);
    const std::uint8_t marker = s.stored_job_valid == Settings::stored_job_marker ? 1u : 0u;
    hi = crc16(&marker, 1, hi);
    std::uint16_t lo = crc16(reinterpret_cast<const std::uint8_t*>(fw), std::strlen(fw));
    lo = crc16(reinterpret_cast<const std::uint8_t*>(&s.counts_per_metre), sizeof s.counts_per_metre, lo);
    return (static_cast<std::uint32_t>(hi) << 16) | static_cast<std::uint32_t>(lo);
}

// ------------------------------------------------------------------ validate
JobError validate(const Job& j) noexcept {
    using namespace ctrl::job;
    if (j.length_um < min_length_um || j.length_um > max_length_um) return JobError::Length;
    if (j.strip_lead_um != 0 && (j.strip_lead_um < min_strip_um || j.strip_lead_um > max_strip_um)) return JobError::StripLead;
    if (j.strip_tail_um != 0 && (j.strip_tail_um < min_strip_um || j.strip_tail_um > max_strip_um)) return JobError::StripTail;
    if (j.wire_diameter_um < wire_min_um || j.wire_diameter_um > wire_max_um) return JobError::WireDiameter;
    if (j.quantity == 0 || j.quantity > max_quantity) return JobError::Quantity;
    const bool strips = j.strip_lead_um != 0 || j.strip_tail_um != 0;
    if (strips) {
        if (j.strip_depth_um < min_strip_depth_um || j.strip_depth_um > max_strip_depth_um) return JobError::Depth;
        if (j.strip_depth_um * 2 >= j.wire_diameter_um) return JobError::Depth; // would sever
        // Ideal per-jaw closure derived from the open apex and the cut radius.
        const double r_mm = (j.wire_diameter_um / 2.0 - j.strip_depth_um) / 1000.0;
        const double closure_mm = blade_open_half_mm - std::sqrt(2.0) * r_mm;
        if (closure_mm < 0.0 || closure_mm > ctrl::axis::blade_cut_mm) return JobError::Geometry;
        // The sever travel is the explicit closed-jaw position plus the sever
        // overlap; it must stay inside the software travel limit.
        if (blade_sever_travel_um > static_cast<std::int32_t>(ctrl::axis::blade_limit_mm * 1000.0)) return JobError::Geometry;
    }
    const std::int64_t grip = static_cast<std::int64_t>(feed::design::minimum_insulated_grip_margin * 1000.0);
    if (static_cast<std::int64_t>(j.strip_lead_um) + j.strip_tail_um + grip > j.length_um) return JobError::Geometry;
    return JobError::None;
}

// ---------------------------------------------------------------- debouncing
void Controller::Debounced::update(bool raw, std::uint64_t now, std::uint32_t debounce_us) {
    if (raw != level) { level = raw; since_us = now; stable = false; return; }
    if (!stable && now - since_us >= debounce_us) {
        stable = true;
        if (level) { pressed_at_us = now; consumed = false; }
    }
}

// --------------------------------------------------------------- construction
Controller::Controller(Board& board) : board_(board) {}

void Controller::init() {
    now_us_ = board_.now_us();
    faults_ = board_.boot_faults();
    state_ = faults_ == 0 ? State::PowerOn : State::Fault;
    phase_ = Phase::Idle;
    homed_ = false;
    out_ = Outputs{};
    out_.drives_disabled = true;
    board_.write_outputs(out_);
    for (auto& a : axis_) a = AxisState{};
    Settings loaded{};
    if (board_.load_settings(&loaded, sizeof loaded) && loaded.magic == Settings{}.magic &&
        loaded.crc == settings_crc(loaded) && loaded.counts_per_metre[0] > 0 && loaded.counts_per_metre[1] > 0 &&
        loaded.counts_per_metre[2] > 0 && signs_valid(loaded) && loaded.part_clear_check <= 1) {
        settings_ = loaded;
        if (settings_.stored_job_valid == Settings::stored_job_marker && validate(settings_.stored_job) == JobError::None) {
            job_ = settings_.stored_job;
            job_loaded_ = true;
        }
    } else {
        settings_ = Settings{};
        settings_.crc = settings_crc(settings_);
    }
    // Fixture mode is RAM-only service state: every boot is production mode.
    fixture_mode_ = false;
    // PowerOn resolves to Fault-free Unhomed only after the first healthy sample.
}

// ------------------------------------------------------------------- helpers
void Controller::set_state(State s) { state_ = s; }
void Controller::set_phase(Phase p) { phase_ = p; phase_started_us_ = now_us_; }
bool Controller::phase_timed_out(std::uint32_t timeout_ms) const { return now_us_ - phase_started_us_ > ms_to_us(timeout_ms); }
bool Controller::inputs_healthy() const {
    if (!safety_.stable || !safety_.level) return false;
    if (!stop_.stable || !stop_.level) return false;
    for (bool a : in_.alm) if (a) return false;
    return true;
}
bool Controller::any_moving() const { for (const auto& a : axis_) if (a.moving) return true; return false; }
bool Controller::all_idle() const {
    for (std::uint8_t i = 0; i < ctrl::axis::count; ++i)
        if (axis_[i].moving || !board_.steps_idle(static_cast<Axis>(i))) return false;
    return true;
}
std::int32_t Controller::blade_travel_um(Axis a) const { return blade_steps_to_um(axis_[a].position_steps); }
std::int32_t Controller::measured_um(std::uint8_t side, std::int64_t counts_delta) const {
    return static_cast<std::int32_t>(std::llround(static_cast<double>(counts_delta) * 1.0e6 / settings_.counts_per_metre[side]));
}
std::int32_t Controller::strip_travel_um() const {
    // F08: ideal per-jaw closure from the open apex, not from the sever travel.
    const double r_mm = (job_.wire_diameter_um / 2.0 - job_.strip_depth_um) / 1000.0;
    const double closure_mm = blade_open_half_mm - std::sqrt(2.0) * r_mm;
    return static_cast<std::int32_t>(std::llround(closure_mm * 1000.0));
}
std::int32_t Controller::length_budget_um(std::int32_t target_um) const {
    // Length target from docs/01 §2: +/- (0.20 mm + 0.05 % of the target).
    const std::int64_t t = target_um < 0 ? -static_cast<std::int64_t>(target_um) : target_um;
    return static_cast<std::int32_t>(200 + (t * 5 + 9999) / 10000);
}
std::int64_t Controller::logical_encoder_counts(std::uint8_t side) const {
    const std::int64_t raw = board_.encoder_count(side);
    return settings_.wheel_sign[side] < 0 ? -raw : raw;
}
std::int64_t Controller::logical_piece_counts() const {
    const std::int64_t raw = board_.piece_encoder_count();
    return settings_.wheel_sign[2] < 0 ? -raw : raw;
}
std::int32_t Controller::piece_station_observed_um(std::int32_t tail0_um, std::int32_t len_um, std::int32_t delta_um) {
    // Occupancy of station D by the moving piece body [tail, tail+len]:
    // observed = { x in [0,|delta|] : tail(x) <= D <= tail(x)+len }.
    // Counts outside that interval are physically zero and never required or
    // fabricated (M27 origin transfer and validity windows).
    const double d_station = ctrl::sensing::piece_station_x_mm * 1000.0;
    const double abs_d = delta_um < 0 ? -static_cast<double>(delta_um) : static_cast<double>(delta_um);
    if (abs_d <= 0.0 || len_um <= 0) return 0;
    const double sign = delta_um >= 0 ? 1.0 : -1.0;
    const double t0 = static_cast<double>(tail0_um);
    const double bound_a = (d_station - static_cast<double>(len_um) - t0) / sign;
    const double bound_b = (d_station - t0) / sign;
    double lo = 0.0, hi = abs_d;
    if (sign > 0) { lo = bound_a > lo ? bound_a : lo; hi = bound_b < hi ? bound_b : hi; }
    else { lo = bound_b > lo ? bound_b : lo; hi = bound_a < hi ? bound_a : hi; }
    if (hi <= lo) return 0;
    return static_cast<std::int32_t>(std::llround(hi - lo));
}
bool Controller::discharge_transition_valid() const {
    // A debounced present edge inside the physical observation window that
    // starts at ejection, followed by a clear edge. A part that cleared before
    // PieceDone still carries valid evidence; a part that never showed present,
    // or never cleared, is quarantined instead of accepted.
    if (!part_present_seen_) return false;
    if (part_present_at_us_ + ms_to_us(ctrl::timing::discharge_window_pre_ms) < clear_started_us_) return false;
    return part_clear_at_us_ > part_present_at_us_;
}
bool Controller::waste_transition_valid() const {
    if (!waste_present_seen_) return false;
    if (waste_present_at_us_ + ms_to_us(ctrl::timing::discharge_window_pre_ms) < waste_started_us_) return false;
    return waste_clear_at_us_ > waste_present_at_us_;
}
void Controller::arm_discharge_window() {
    // Observation window starts at ejection; evidence is captured from here on.
    // A body already presenting at window start (a long piece spans the
    // discharge zone before ejection) counts as present evidence; only the
    // clear edge must then be observed.
    clear_started_us_ = now_us_;
    part_present_seen_ = !in_.part_clear;
    part_present_at_us_ = part_present_seen_ ? now_us_ : 0;
    part_clear_at_us_ = 0;
}
void Controller::begin_waste_sweep() {
    waste_present_seen_ = false;
    waste_present_at_us_ = 0;
    waste_clear_at_us_ = 0;
    waste_started_us_ = now_us_;
    out_.waste_sweep = true;
    set_phase(Phase::WasteSweep);
}

void Controller::finish_waste() {
    using namespace ctrl;
    const Axis R = axis::feed_right;
    switch (waste_next_) {
    case WasteNext::PrimeDone:
        piece_done_prime_ = priming_;
        priming_ = false;
        piece_present_ = false;
        complete_or_launch();
        break;
    case WasteNext::AfterLead:
        start_length_feed();
        break;
    case WasteNext::AfterTail:
        start_tracked_feed(R, job::ejection_um, axis::feed_speed_mm_s, axis::feed_accel_mm_s2, 0, true);
        set_phase(Phase::Eject);
        break;
    }
}
bool Controller::wire_state_valid() const {
    if (piece_present_) {
        if (piece_len_um_ <= 0) return false;
        if (!wire::bodies_valid(stock_tip_um_, piece_tail_um_, piece_present_)) return false; // overlap
    }
    return true;
}
bool Controller::approved() const noexcept {
    return settings_.approval == Settings::approval_marker && settings_.approval_key != 0 &&
           settings_.approval_key == commissioning_identity(settings_);
}

// --------------------------------------------------------------------- tick
void Controller::tick() {
    now_us_ = board_.now_us();
    sample_inputs();
    supervise_safety();

    switch (state_) {
    case State::PowerOn:
        if (inputs_healthy()) set_state(State::Unhomed);
        break;
    case State::Fault:
    case State::Unhomed:
    case State::Ready:
    case State::Paused:
        break;
    case State::Homing:
        if (raw_.safety_ok) run_homing();
        break;
    case State::Running:
        if (raw_.safety_ok) run_cycle();
        break;
    case State::Jogging:
        if (!raw_.safety_ok) break;
        if (phase_ == Phase::EnableSettle) {
            if (now_us_ - drives_enabled_at_us_ >= ms_to_us(ctrl::timing::drive_enable_settle_ms)) {
                if (!begin_jog_move()) { enter_fault(fault::internal); break; }
                set_phase(Phase::Jog);
            }
            break;
        }
        service_axes();
        if (all_idle()) {
            axis_[0].moving = axis_[1].moving = axis_[2].moving = axis_[3].moving = false;
            if (commissioning_) { finish_commissioning(); break; }
            set_phase(Phase::Idle);
            set_state(homed_ ? State::Ready : State::Unhomed);
        } else if (phase_timed_out(ctrl::timing::move_timeout_ms)) enter_fault(fault::move_timeout);
        break;
    case State::Stopping:
        if (!raw_.safety_ok) break;
        service_axes();
        if (all_idle()) {
            for (auto& a : axis_) a.moving = false;
            set_phase(Phase::Idle);
            if (stop_return_state_ == State::Paused) ++counters_.pieces_scrap;
            set_state(stop_return_state_);
        }
        break;
    }

    handle_buttons();
    update_indicators();
    board_.write_outputs(out_);
    board_.watchdog_feed();
}

void Controller::pump() {
    if (!raw_.safety_ok) return;
    if (state_ == State::Running || state_ == State::Homing || state_ == State::Jogging || state_ == State::Stopping)
        service_axes();
}

void Controller::sample_inputs() {
    raw_ = board_.read_inputs();
    const std::uint32_t db = ctrl::timing::input_debounce_ms * 1000;
    start_.update(raw_.start, now_us_, db);
    stop_.update(raw_.stop_ok, now_us_, db);
    reset_.update(raw_.reset, now_us_, db);
    safety_.update(raw_.safety_ok, now_us_, db);
    home_front_.update(raw_.home_front, now_us_, db);
    home_rear_.update(raw_.home_rear, now_us_, db);
    part_clear_.update(raw_.part_clear, now_us_, db);
    waste_clear_.update(raw_.waste_clear, now_us_, db);
    in_ = raw_;
    in_.start = start_.stable && start_.level;
    in_.stop_ok = stop_.stable && stop_.level;
    in_.reset = reset_.stable && reset_.level;
    in_.safety_ok = safety_.stable && safety_.level;
    in_.home_front = home_front_.stable && home_front_.level;
    in_.home_rear = home_rear_.stable && home_rear_.level;
    in_.part_clear = part_clear_.stable && part_clear_.level;
    in_.waste_clear = waste_clear_.stable && waste_clear_.level;
    // Capture debounced sensor edges continuously: the discharge present->clear
    // evidence must survive transitions that complete before PieceDone, and the
    // waste transit that completes inside the sweep window (M27/M30).
    if (in_.part_clear != part_level_last_) {
        part_level_last_ = in_.part_clear;
        if (in_.part_clear) part_clear_at_us_ = now_us_;
        else { part_present_seen_ = true; part_present_at_us_ = now_us_; }
    }
    if (in_.waste_clear != waste_level_last_) {
        waste_level_last_ = in_.waste_clear;
        if (in_.waste_clear) waste_clear_at_us_ = now_us_;
        else { waste_present_seen_ = true; waste_present_at_us_ = now_us_; }
    }
}

void Controller::supervise_safety() {
    // Raw safety loss stops pulses immediately; the latched fault follows debounce.
    if (!raw_.safety_ok) {
        if (any_moving()) abort_all();
        out_.drives_disabled = true;
    }
    if (state_ == State::PowerOn) return;
    if (safety_.stable && !safety_.level && state_ != State::Fault) enter_fault(fault::safety_open);
    for (std::uint8_t i = 0; i < ctrl::axis::count; ++i)
        if (raw_.alm[i] && state_ != State::Fault) enter_fault(alm_bit(static_cast<Axis>(i)));
    const FaultBits board_faults = board_.latched_faults();
    if (board_faults != 0 && state_ != State::Fault) enter_fault(board_faults);
    for (std::uint8_t i = 0; i < 2; ++i)
        if (!board_.encoder_healthy(i) && state_ != State::Fault) enter_fault(fault::encoder_overflow);
    // STOP loop open while in motion: controlled stop (not a fault).
    if (stop_.stable && !stop_.level) {
        if (state_ == State::Running) begin_controlled_stop(State::Paused);
        else if (state_ == State::Homing) begin_controlled_stop(State::Unhomed);
        else if (state_ == State::Jogging) begin_controlled_stop(homed_ ? State::Ready : State::Unhomed);
    }
}

void Controller::enter_fault(FaultBits bits) {
    abort_all();
    out_.drives_disabled = true;
    faults_ |= bits;
    if (state_ == State::Running) ++counters_.pieces_scrap;
    homed_ = false;
    set_phase(Phase::Idle);
    set_state(State::Fault);
    beep_until_us_ = now_us_ + ms_to_us(ctrl::timing::buzzer_fault_ms);
}

void Controller::abort_all() {
    for (std::uint8_t i = 0; i < ctrl::axis::count; ++i) {
        board_.abort_steps(static_cast<Axis>(i));
        axis_[i].moving = false;
        axis_[i].streamer = StepStreamer{};
    }
}

void Controller::begin_controlled_stop(State return_state) {
    if (state_ == State::Stopping) return;
    stop_return_state_ = return_state;
    for (auto& a : axis_) if (a.moving) a.streamer.begin_stop();
    set_phase(Phase::Stop);
    set_state(State::Stopping);
}

// ------------------------------------------------------------------- buttons
void Controller::handle_buttons() {
    // RESET: held ctrl::timing::reset_hold_ms.
    if (reset_.stable && reset_.level && !reset_.consumed &&
        now_us_ - reset_.pressed_at_us >= ms_to_us(ctrl::timing::reset_hold_ms)) {
        reset_.consumed = true;
        (void)reset();
    }
    if (start_.stable && start_.level && !start_.consumed &&
        now_us_ - start_.pressed_at_us >= ms_to_us(ctrl::timing::start_hold_ms)) {
        start_.consumed = true;
        (void)start();
    }
}

void Controller::update_indicators() {
    const bool slow = ((now_us_ / ms_to_us(ctrl::timing::blink_slow_ms)) & 1u) != 0;
    const bool fast = ((now_us_ / ms_to_us(ctrl::timing::blink_fast_ms)) & 1u) != 0;
    out_.led_red = false; out_.led_green = false;
    switch (state_) {
    case State::PowerOn:  out_.led_red = slow; break;
    case State::Fault:    out_.led_red = true; break;
    case State::Unhomed:  out_.led_red = slow; out_.led_green = job_loaded_ ? slow : false; break;
    case State::Homing:   out_.led_green = fast; break;
    case State::Ready:    out_.led_green = job_loaded_ ? slow : false; break;
    case State::Running:  out_.led_green = true; break;
    case State::Stopping: out_.led_green = fast; out_.led_red = fast; break;
    case State::Paused:   out_.led_green = slow; out_.led_red = slow; break;
    case State::Jogging:  out_.led_green = fast; break;
    }
    out_.buzzer = now_us_ < beep_until_us_;
    if (state_ == State::Fault || state_ == State::PowerOn) out_.drives_disabled = true;
}

// ------------------------------------------------------------------- motion
bool Controller::start_move(Axis a, std::int64_t delta_steps, double speed_steps_s, double accel_steps_s2,
                            std::uint16_t stall_fault, std::uint32_t max_run) {
    AxisState& ax = axis_[a];
    if (ax.moving || delta_steps == 0) return delta_steps == 0;
    if (delta_steps > 0xFFFFFFFFll || delta_steps < -0xFFFFFFFFll) return false;
    MoveSpec spec;
    spec.steps = static_cast<std::uint32_t>(delta_steps < 0 ? -delta_steps : delta_steps);
    spec.steps_per_s = speed_steps_s;
    spec.steps_per_s2 = accel_steps_s2;
    spec.forward = delta_steps > 0;
    if (!ax.streamer.begin(spec, ctrl::axis::pio_clock_hz, max_run)) return false;
    // F33: the commissioned per-axis polarity maps the logical direction onto
    // the physical DIR line; position tracking stays logical.
    board_.set_direction(a, spec.forward != (settings_.axis_sign[a] < 0));
    ax.moving = true;
    ax.stall_fault = stall_fault;
    ax.queue_limit = max_run == 1 ? 1 : Board::step_fifo_depth;
    ax.move_start_us = now_us_;
    // Stall supervision reads the physically observing station: the stock
    // wheels for stock moves, station D while the right cassette drives the
    // severed piece.
    ax.encoder_start = (a == ctrl::axis::feed_right && piece_present_)
                           ? logical_piece_counts()
                           : logical_encoder_counts(a == ctrl::axis::feed_right ? 1 : 0);
    return true;
}

bool Controller::start_blade_moves_um(std::int32_t target_travel_um, double speed_mm_s, double accel_mm_s2, std::uint32_t max_run) {
    if (target_travel_um < 0 || target_travel_um > static_cast<std::int32_t>(ctrl::axis::blade_limit_mm * 1000.0)) {
        enter_fault(fault::blade_limit);
        return false;
    }
    const std::int64_t target = um_to_blade_steps(target_travel_um);
    bool ok = true;
    for (Axis a : {ctrl::axis::blade_front, ctrl::axis::blade_rear}) {
        const std::int64_t delta = target - axis_[a].position_steps;
        ok = start_move(a, delta, speed_mm_s * ctrl::axis::blade_steps_per_mm, accel_mm_s2 * ctrl::axis::blade_steps_per_mm, 0, max_run) && ok;
    }
    return ok;
}

bool Controller::start_feed_move_um(Axis a, std::int32_t delta_um, double speed_mm_s, double accel_mm_s2, std::uint16_t stall_fault) {
    return start_move(a, um_to_feed_steps(delta_um), speed_mm_s * ctrl::axis::feed_steps_per_mm,
                      accel_mm_s2 * ctrl::axis::feed_steps_per_mm, stall_fault);
}

void Controller::start_tracked_feed(Axis a, std::int32_t delta_um, double speed_mm_s, double accel_mm_s2,
                                    std::uint16_t stall_fault, bool final_eject) {
    // Physical observability plan (M27): the stock body is observed by the two
    // stock-side wheels; the severed piece only by downstream station D while
    // it occupies that station. A body the right cassette cannot grip is never
    // moved by feed commands at all (see the waste sweep).
    const bool body_is_piece = a == ctrl::axis::feed_right && piece_present_;
    TrackedFeed& t = body_is_piece ? tracked_piece_ : tracked_[a == ctrl::axis::feed_right ? 1 : 0];
    t.expect_um = delta_um;
    t.body_is_piece = body_is_piece;
    t.final_eject = body_is_piece && final_eject;
    t.active = true;
    if (body_is_piece) {
        t.encoder_start = logical_piece_counts();
        t.observed_expect_um = piece_station_observed_um(piece_tail_um_, piece_len_um_, delta_um);
        // Unobserved piece transport is legal only for the final ejection,
        // where the discharge present->clear evidence covers the remainder
        // before acceptance. Any other piece move must be fully observable at
        // D; a hole here means the sensor architecture is inconsistent.
        const std::int32_t magnitude = delta_um < 0 ? -delta_um : delta_um;
        if (!t.final_eject && t.observed_expect_um < magnitude) {
            enter_fault(fault::wire_geometry);
            return;
        }
        if (t.final_eject) arm_discharge_window();
    } else {
        const std::uint8_t side = a == ctrl::axis::feed_right ? 1 : 0;
        t.encoder_start = logical_encoder_counts(side);
        t.observed_expect_um = delta_um < 0 ? -delta_um : delta_um;
    }
    if (!start_feed_move_um(a, delta_um, speed_mm_s, accel_mm_s2, stall_fault)) enter_fault(fault::internal);
}

bool Controller::verify_tracked_feed(std::uint8_t side, bool gross_only) {
    // Stock body at wheel W0/W1: both wheels ride continuous stock, so the
    // commanded stock transport is fully observable (no occupancy window).
    TrackedFeed& t = tracked_[side];
    if (!t.active) return true;
    t.active = false;
    const std::int32_t moved = measured_um(side, logical_encoder_counts(side) - t.encoder_start);
    const std::int64_t residual = static_cast<std::int64_t>(t.expect_um) - moved;
    const std::int64_t magnitude = residual < 0 ? -residual : residual;
    if (magnitude > ctrl::enc::max_slip_um) {
        enter_fault(fault::length_slip); // gross slip: motion fault threshold
        return false;
    }
    if (!gross_only) {
        // Strip/pull moves use the same 0.20 mm + 0.05 % budget shape as the
        // piece length, with the strip tolerance as the base term.
        const std::int32_t budget = length_budget_um(t.expect_um);
        if (magnitude > budget) {
            enter_fault(fault::length_budget); // piece/strip acceptance budget
            return false;
        }
    }
    return true;
}

bool Controller::verify_tracked_piece(bool gross_only) {
    // Severed piece at downstream station D: compare exactly the observable
    // portion of the move (signed; the piece may advance or backfeed). Counts
    // outside the occupancy interval are physically zero; the unobserved
    // remainder of a final ejection is covered by the discharge evidence
    // before the piece can be accepted.
    TrackedFeed& t = tracked_piece_;
    if (!t.active) return true;
    t.active = false;
    if (!board_.piece_encoder_healthy()) { enter_fault(fault::encoder_invalid); return false; }
    const std::int32_t moved = measured_um(2, logical_piece_counts() - t.encoder_start);
    const std::int32_t observed = t.observed_expect_um;
    const std::int32_t moved_magnitude = moved < 0 ? -moved : moved;
    if (moved_magnitude > observed + static_cast<std::int32_t>(2 * ctrl::enc::nominal_um_per_count)) {
        // Motion recorded outside the planned occupancy: geometry or sensor
        // model disagreement. Never accept it silently.
        enter_fault(fault::wire_geometry);
        return false;
    }
    const std::int32_t expected_signed = t.expect_um < 0 ? -observed : observed;
    const std::int64_t residual = static_cast<std::int64_t>(expected_signed) - moved;
    const std::int64_t magnitude = residual < 0 ? -residual : residual;
    if (magnitude > ctrl::enc::max_slip_um) {
        enter_fault(fault::length_slip);
        return false;
    }
    if (!gross_only) {
        const std::int32_t budget = length_budget_um(t.expect_um);
        if (magnitude > budget) {
            enter_fault(fault::length_budget);
            return false;
        }
    }
    return true;
}

void Controller::service_axes() {
    if (out_.drives_disabled) return;
    for (std::uint8_t i = 0; i < ctrl::axis::count; ++i) {
        const Axis a = static_cast<Axis>(i);
        AxisState& ax = axis_[i];
        if (!ax.moving) continue;
        while (ax.streamer.active() && board_.step_fifo_level(a) < ax.queue_limit) {
            StepRun run{};
            if (!ax.streamer.pop(run)) break;
            board_.push_steps(a, run);
            ax.position_steps += ax.streamer.forward() ? run.count : -static_cast<std::int64_t>(run.count);
        }
        if (!ax.streamer.active() && board_.steps_idle(a)) ax.moving = false;
        if (ax.stall_fault != 0 && ax.moving) {
            const std::int64_t issued_um = feed_steps_to_um(ax.streamer.issued());
            const std::int64_t enc = (a == ctrl::axis::feed_right && piece_present_)
                                         ? logical_piece_counts() - ax.encoder_start
                                         : logical_encoder_counts(a == ctrl::axis::feed_right ? 1 : 0) - ax.encoder_start;
            const std::int32_t moved_um = std::abs(measured_um(a == ctrl::axis::feed_right && piece_present_ ? 2 : (a == ctrl::axis::feed_right ? 1 : 0), enc));
            if (issued_um >= 5000 && moved_um < 1000) { enter_fault(ax.stall_fault); return; }
        }
        if (ax.moving && now_us_ - ax.move_start_us > ms_to_us(ctrl::timing::move_timeout_ms)) { enter_fault(fault::move_timeout); return; }
    }
}

// ------------------------------------------------------------------- homing
void Controller::begin_homing() {
    for (auto& a : axis_) a.moving = false;
    home_sub_done_[0] = home_sub_done_[1] = false;
    // Homing is part of the documented recovery/rethread sequence (M37): the
    // operator has cleared and rethreaded the wire lane, so the wire-body
    // model returns to the known threaded state at the plane.
    stock_tip_um_ = 0;
    piece_tail_um_ = 0;
    piece_len_um_ = 0;
    piece_present_ = false;
    stock_restore_um_ = 0;
    out_.waste_sweep = false;
    out_.drives_disabled = false;
    drives_enabled_at_us_ = now_us_;
    homing_started_us_ = now_us_;
    set_state(State::Homing);
    set_phase(Phase::EnableSettle);
}

void Controller::run_homing() {
    using namespace ctrl::timing;
    const double slow = homing_slow_mm_s, fast = homing_fast_mm_s, acc = ctrl::axis::blade_accel_mm_s2;
    const double bsm = ctrl::axis::blade_steps_per_mm;
    const bool sensor[2] = {in_.home_front, in_.home_rear};
    const Axis blades[2] = {ctrl::axis::blade_front, ctrl::axis::blade_rear};
    service_axes();
    if (state_ != State::Homing) return;
    // The 20 s contract covers the complete reference operation, not each
    // individual leave/seek/backoff/approach phase.
    if (now_us_ - homing_started_us_ > ms_to_us(homing_timeout_ms)) { enter_fault(fault::homing_failed); return; }

    switch (phase_) {
    case Phase::EnableSettle:
        if (now_us_ - drives_enabled_at_us_ >= ms_to_us(drive_enable_settle_ms)) {
            // Leave the sensor if already on it (move toward the wire by the backoff).
            for (int i = 0; i < 2; ++i) {
                home_sub_done_[i] = !sensor[i];
                if (sensor[i]) start_move(blades[i], um_to_blade_steps(static_cast<std::int64_t>(homing_backoff_mm * 1000)), slow * bsm, acc * bsm);
            }
            set_phase(Phase::HomeLeave);
        }
        break;
    case Phase::HomeLeave:
        if (!any_moving()) {
            for (int i = 0; i < 2; ++i) {
                if (sensor[i]) { enter_fault(fault::homing_failed); return; } // still on sensor after leaving
                home_sub_done_[i] = false;
                start_move(blades[i], -um_to_blade_steps(static_cast<std::int64_t>(homing_search_mm * 1000)), fast * bsm, acc * bsm);
            }
            set_phase(Phase::HomeSeek);
        }
        break;
    case Phase::HomeSeek:
        for (int i = 0; i < 2; ++i) {
            if (home_sub_done_[i]) continue;
            if (sensor[i]) { board_.abort_steps(blades[i]); axis_[blades[i]].moving = false; axis_[blades[i]].streamer = StepStreamer{}; home_sub_done_[i] = true; }
            else if (!axis_[blades[i]].moving) { enter_fault(fault::homing_failed); return; }
        }
        if (home_sub_done_[0] && home_sub_done_[1]) {
            for (int i = 0; i < 2; ++i) start_move(blades[i], um_to_blade_steps(static_cast<std::int64_t>(homing_backoff_mm * 1000)), slow * bsm, acc * bsm);
            set_phase(Phase::HomeBackoff);
        }
        break;
    case Phase::HomeBackoff:
        if (!any_moving()) {
            for (int i = 0; i < 2; ++i) {
                if (sensor[i]) { enter_fault(fault::homing_failed); return; }
                home_sub_done_[i] = false;
                start_move(blades[i], -um_to_blade_steps(static_cast<std::int64_t>(2.0 * homing_backoff_mm * 1000)), slow * bsm, acc * bsm, 0, 1);
            }
            set_phase(Phase::HomeApproach);
        }
        break;
    case Phase::HomeApproach:
        for (int i = 0; i < 2; ++i) {
            if (home_sub_done_[i]) continue;
            if (sensor[i]) {
                board_.abort_steps(blades[i]);
                axis_[blades[i]].moving = false;
                axis_[blades[i]].streamer = StepStreamer{};
                axis_[blades[i]].position_steps = 0;
                home_sub_done_[i] = true;
            } else if (!axis_[blades[i]].moving) { enter_fault(fault::homing_failed); return; }
        }
        if (home_sub_done_[0] && home_sub_done_[1]) {
            homed_ = true;
            set_phase(Phase::Idle);
            set_state(State::Ready);
        }
        break;
    default:
        enter_fault(fault::internal);
    }
}

// -------------------------------------------------------------------- cycle
void Controller::begin_job_run() {
    // A completed batch remains the active stored recipe. Starting from Ready
    // begins a fresh batch; restarting from Paused retains its current count.
    if (state_ == State::Ready) counters_ = Counters{};
    priming_ = true;
    correction_done_ = false;
    // The wire-body model is physical state: it is never blindly reset here.
    // A leftover severed piece and a long prime-severed tip are handled by
    // launch_pass/Cut (scrap discharge or stub sweep); the model is re-
    // initialised only by an explicit rethread (home()).
    part_present_seen_ = false;
    part_clear_confirmed_ = false;
    part_present_at_us_ = 0;
    part_clear_at_us_ = 0;
    waste_present_seen_ = false;
    waste_clear_confirmed_ = false;
    waste_present_at_us_ = 0;
    waste_clear_at_us_ = 0;
    out_.waste_sweep = false;
    out_.drives_disabled = false;
    drives_enabled_at_us_ = now_us_;
    set_state(State::Running);
    set_phase(Phase::EnableSettle);
}

void Controller::launch_pass() {
    if (priming_) {
        if (piece_present_) {
            // Physical recovery (M37): an interrupted pass left a severed piece
            // in the machine. Discharge it as scrap through the takeaway with
            // the full evidence chain before any new feed. The stock cut is
            // already square, so no prime trim follows (finalize_piece clears
            // priming_ and the next pass is production).
            start_tracked_feed(ctrl::axis::feed_right, ctrl::job::ejection_um, ctrl::axis::feed_speed_mm_s,
                               ctrl::axis::feed_accel_mm_s2, 0, true);
            set_phase(Phase::Eject);
            return;
        }
        // Prime pass: feed the trimmed stock far enough past the cut plane,
        // sever the stub and discharge it. It does not count a piece.
        feed_encoder_start_[0] = logical_encoder_counts(0);
        feed_encoder_start_[1] = logical_encoder_counts(1);
        feed_target_um_ = spec::stock_clear_um;
        if (!start_feed_move_um(ctrl::axis::feed_left, feed_target_um_, ctrl::axis::feed_speed_mm_s,
                                ctrl::axis::feed_accel_mm_s2, fault::no_wire)) enter_fault(fault::internal);
        if (!start_feed_move_um(ctrl::axis::feed_right, feed_target_um_, ctrl::axis::feed_speed_mm_s,
                                ctrl::axis::feed_accel_mm_s2)) enter_fault(fault::internal);
        stock_tip_um_ += feed_target_um_;
        set_phase(Phase::Feed);
        return;
    }
    correction_done_ = false;
    if (job_.strip_lead_um > 0) {
        // docs/01 §4 step 1: advance the stock by s1 so the score plane sits s1
        // behind the trimmed tip.
        start_tracked_feed(ctrl::axis::feed_left, job_.strip_lead_um, ctrl::axis::strip_speed_mm_s, ctrl::axis::strip_accel_mm_s2);
        stock_tip_um_ += job_.strip_lead_um;
        set_phase(Phase::LeadAdvance);
        return;
    }
    start_length_feed();
}

void Controller::start_length_feed() {
    correction_done_ = false;
    feed_encoder_start_[0] = logical_encoder_counts(0);
    feed_encoder_start_[1] = logical_encoder_counts(1);
    // docs/01 §4 step 2: feed L+3 after a lead strip (the tip is 3 mm behind the
    // plane after the lead retract), or L when no lead strip is configured.
    feed_target_um_ = job_.length_um + (job_.strip_lead_um > 0 ? strip_clear_um : 0);
    if (!start_feed_move_um(ctrl::axis::feed_left, feed_target_um_, ctrl::axis::feed_speed_mm_s,
                            ctrl::axis::feed_accel_mm_s2, fault::encoder_stalled)) enter_fault(fault::internal);
    if (!start_feed_move_um(ctrl::axis::feed_right, feed_target_um_, ctrl::axis::feed_speed_mm_s,
                            ctrl::axis::feed_accel_mm_s2)) enter_fault(fault::internal);
    stock_tip_um_ += feed_target_um_;
    set_phase(Phase::Feed);
}

bool Controller::begin_jog_move() {
    const bool blade = jog_axis_ == ctrl::axis::blade_front || jog_axis_ == ctrl::axis::blade_rear;
    if (blade) {
        // F33: widened checked arithmetic before any range decision.
        const std::int64_t current = blade_travel_um(jog_axis_);
        const std::int64_t delta = jog_forward_ ? static_cast<std::int64_t>(jog_distance_um_) : -static_cast<std::int64_t>(jog_distance_um_);
        const std::int64_t target = current + delta;
        if (target < 0 || target > static_cast<std::int64_t>(ctrl::axis::blade_limit_mm * 1000.0)) {
            enter_fault(fault::blade_limit);
            return false;
        }
        const double speed = std::min(jog_speed_um_s_ / 1000.0, ctrl::axis::blade_speed_mm_s);
        return start_move(jog_axis_, um_to_blade_steps(target) - axis_[jog_axis_].position_steps,
                          speed * ctrl::axis::blade_steps_per_mm,
                          ctrl::axis::blade_accel_mm_s2 * ctrl::axis::blade_steps_per_mm);
    }
    const double speed = std::min(jog_speed_um_s_ / 1000.0, ctrl::axis::feed_speed_mm_s);
    return start_feed_move_um(jog_axis_, jog_forward_ ? jog_distance_um_ : -jog_distance_um_, speed,
                              ctrl::axis::feed_accel_mm_s2);
}

void Controller::run_cycle() {
    using namespace ctrl;
    const Axis L = axis::feed_left, R = axis::feed_right;
    service_axes();
    if (state_ != State::Running) return;

    switch (phase_) {
    case Phase::EnableSettle:
        if (now_us_ - drives_enabled_at_us_ >= ms_to_us(timing::drive_enable_settle_ms)) {
            if (blade_travel_um(axis::blade_front) != 0 || blade_travel_um(axis::blade_rear) != 0) {
                start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
                set_phase(Phase::Open);
            } else launch_pass();
        }
        break;
    case Phase::Open: // blades finished opening before the first pass
        if (!any_moving()) launch_pass();
        break;
    case Phase::Feed:
        if (!any_moving()) {
            if (priming_) { // the prime feed is a threading move, not an accepted piece
                start_blade_moves_um(blade_sever_travel_um, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
                set_phase(Phase::Cut);
            } else {
                set_phase(Phase::FeedVerify);
            }
        }
        break;
    case Phase::FeedVerify: {
        if (!board_.encoder_healthy(0) || !board_.encoder_healthy(1)) { enter_fault(fault::encoder_invalid); return; }
        const std::int32_t residual[2] = {
            feed_target_um_ - measured_um(0, logical_encoder_counts(0) - feed_encoder_start_[0]),
            feed_target_um_ - measured_um(1, logical_encoder_counts(1) - feed_encoder_start_[1]),
        };
        counters_.last_residual_um = residual[0];
        for (std::int32_t r : residual)
            if (std::abs(r) > std::abs(counters_.worst_residual_um)) counters_.worst_residual_um = r;
        // Correction deadband and the wheel-disagreement guard allow for two
        // independent count quantisations plus one accumulated fraction.
        const std::int32_t deadband = static_cast<std::int32_t>(2.0 * enc::nominal_um_per_count);
        const std::int32_t disagreement_um = static_cast<std::int32_t>(std::ceil(3.0 * enc::nominal_um_per_count));
        if (std::abs(residual[0]) > enc::max_slip_um || std::abs(residual[1]) > enc::max_slip_um ||
            std::abs(residual[0] - residual[1]) > disagreement_um) {
            enter_fault(fault::length_slip); // gross motion threshold
            return;
        }
        const std::int32_t correction = static_cast<std::int32_t>(
            std::llround((static_cast<double>(residual[0]) + residual[1]) / 2.0));
        if (!correction_done_ && std::abs(correction) > deadband) {
            correction_done_ = true;
            // One bounded correction move; gross-verified here so that the
            // recheck below still applies the recipe acceptance budget.
            start_tracked_feed(L, correction, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            start_tracked_feed(R, correction, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            stock_tip_um_ += correction;
            set_phase(Phase::FeedCorrect);
        } else {
            // Acceptance budget (docs/01 §2): 0.20 mm + 0.05 % of the target.
            // A piece outside it is quarantined by the fault and never counted.
            const std::int32_t budget = length_budget_um(feed_target_um_);
            if (std::abs(residual[0]) > budget || std::abs(residual[1]) > budget) {
                enter_fault(fault::length_budget);
                return;
            }
            start_blade_moves_um(blade_sever_travel_um, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::Cut);
        }
        break;
    }
    case Phase::FeedCorrect:
        if (!any_moving()) {
            if (!verify_tracked_feed(0, true) || !verify_tracked_feed(1, true)) return;
            set_phase(Phase::FeedVerify);
        }
        break;
    case Phase::Cut:
        if (!any_moving()) {
            // Severed: the piece tail and the stock tip both sit on the cut
            // plane. The prime pass severs the actual modelled tip: a short
            // body (fresh 10 mm stub) is waste for the sweep; a long body left
            // by an interrupted feed is scrap discharged through the takeaway.
            piece_present_ = true;
            piece_tail_um_ = 0;
            piece_len_um_ = priming_ ? stock_tip_um_ : job_.length_um;
            stock_tip_um_ = 0;
            if (!wire_state_valid()) { enter_fault(fault::wire_geometry); return; }
            start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::OpenAfterCut);
        }
        break;
    case Phase::OpenAfterCut:
        if (!any_moving()) {
            if (priming_ && piece_len_um_ < static_cast<std::int32_t>(ctrl::sensing::piece_body_grip_x_mm * 1000.0)) {
                // The severed prime stub (10 mm fresh) cannot reach the nearest
                // right nip at 75.5 mm and is never propelled by feed motion.
                // It is removed by the constrained mechanical sweep and
                // verified through the waste chute (M30).
                waste_next_ = WasteNext::PrimeDone;
                begin_waste_sweep();
            } else if (priming_) {
                // A long prime-severed body (stock left by an interrupted
                // feed) is grippable scrap: discharge it through the takeaway
                // with the full handling evidence chain, uncounted (the
                // interruption already counted its scrap).
                start_tracked_feed(R, job::ejection_um, axis::feed_speed_mm_s, axis::feed_accel_mm_s2, 0, true);
                set_phase(Phase::Eject);
            } else if (job_.strip_tail_um == 0) {
                start_tracked_feed(R, job::ejection_um, axis::feed_speed_mm_s, axis::feed_accel_mm_s2, 0, true);
                set_phase(Phase::Eject);
            } else {
                // docs/01 §4 step 4: withdraw stock s2+10 and backfeed the piece
                // s2, opening a 10 mm separation before the tail score.
                const std::int32_t withdraw = job_.strip_tail_um + wire::separation_um;
                start_tracked_feed(L, -withdraw, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
                start_tracked_feed(R, -job_.strip_tail_um, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
                stock_tip_um_ -= withdraw;
                piece_tail_um_ -= job_.strip_tail_um;
                stock_restore_um_ = withdraw;
                if (!wire_state_valid()) { enter_fault(fault::wire_geometry); return; }
                set_phase(Phase::TailWithdraw);
            }
        }
        break;
    case Phase::TailWithdraw:
        if (!any_moving()) {
            if (!verify_tracked_feed(0, false) || !verify_tracked_piece(false)) return;
            // Reject scoring the wrong end (piece must be at the plane), any
            // loss of retained grip and any stock/piece overlap (docs/01 §4).
            if (!wire_state_valid() ||
                !wire::tail_score_valid(stock_tip_um_, piece_tail_um_, piece_len_um_,
                                        job_.strip_lead_um, job_.strip_tail_um, piece_present_)) {
                enter_fault(fault::wire_geometry);
                return;
            }
            start_blade_moves_um(strip_travel_um(), axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::TailScore);
        }
        break;
    case Phase::TailScore:
        if (!any_moving()) {
            start_tracked_feed(R, job_.strip_tail_um + strip_clear_um, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            piece_tail_um_ += job_.strip_tail_um + strip_clear_um;
            set_phase(Phase::TailPull);
        }
        break;
    case Phase::TailPull:
        if (!any_moving()) {
            if (!verify_tracked_piece(false)) return;
            start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::TailOpen);
        }
        break;
    case Phase::TailOpen:
        if (!any_moving()) {
            // The released tail slug is swept into the waste chute and verified
            // before the ejection can trap it against the takeaway.
            waste_next_ = WasteNext::AfterTail;
            begin_waste_sweep();
        }
        break;
    case Phase::Eject:
        if (!any_moving()) {
            if (!verify_tracked_piece(true)) return; // the part intentionally leaves the nip
            piece_present_ = false;
            set_phase(Phase::PieceDone);
        }
        break;
    case Phase::PieceDone:
        // PieceDone counts the physical completion; accepted-pieces is a
        // separate count that requires the complete handling evidence chain.
        // The discharge observation window was armed at ejection and its edge
        // history is NOT restarted here: a part that presented and cleared
        // before this phase still carries valid evidence (M27).
        piece_done_prime_ = priming_;
        if (!priming_) ++counters_.pieces_done;
        if (fixture_mode_ || !part_clear_check_enabled()) {
            // Fixture/engineering mode: no handling verification is claimed and
            // no accepted production count is emitted.
            part_clear_confirmed_ = true;
            finalize_piece();
        } else {
            set_phase(Phase::AwaitClear);
        }
        break;
    case Phase::AwaitClear:
        if (discharge_transition_valid()) {
            part_clear_confirmed_ = true;
            finalize_piece();
        } else if (phase_timed_out(ctrl::timing::discharge_clear_timeout_ms)) {
            enter_fault(fault::part_not_cleared); // quarantined: no accepted count
        }
        break;
    case Phase::WasteSweep:
        if (phase_timed_out(ctrl::timing::waste_sweep_ms)) {
            out_.waste_sweep = false;
            set_phase(Phase::WasteClear);
        }
        break;
    case Phase::WasteClear:
        if (fixture_mode_ || !part_clear_check_enabled()) {
            // Fixture/engineering mode: no handling verification is claimed and
            // no accepted counts are emitted; runs may proceed on incomplete
            // hardware. Production demands the chute transit.
            waste_clear_confirmed_ = true;
            finish_waste();
        } else if (waste_transition_valid()) {
            waste_clear_confirmed_ = true;
            finish_waste();
        } else if (phase_timed_out(ctrl::timing::waste_clear_timeout_ms)) {
            enter_fault(fault::waste_not_cleared); // a trapped stub/slug must not be fed over
        }
        break;
    case Phase::RestoreStock:
        if (!any_moving()) {
            if (!verify_tracked_feed(0, false)) return;
            complete_or_launch();
        }
        break;
    case Phase::LeadAdvance:
        if (!any_moving()) {
            if (!verify_tracked_feed(0, false)) return;
            // Reject scoring the wrong end: the stock tip must sit s1 past the
            // plane and no severed piece may occupy it.
            if (!wire::lead_score_valid(stock_tip_um_, job_.strip_lead_um, piece_present_)) {
                enter_fault(fault::wire_geometry);
                return;
            }
            start_blade_moves_um(strip_travel_um(), axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::LeadScore);
        }
        break;
    case Phase::LeadScore:
        if (!any_moving()) {
            // While the blades retain the slug, retract the stock s1+3; the bare
            // tip ends 3 mm behind the plane (docs/01 §4 step 1).
            const std::int32_t retract = job_.strip_lead_um + strip_clear_um;
            start_tracked_feed(L, -retract, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            stock_tip_um_ -= retract;
            set_phase(Phase::LeadRetract);
        }
        break;
    case Phase::LeadRetract:
        if (!any_moving()) {
            if (!verify_tracked_feed(0, false)) return;
            if (!wire_state_valid()) { enter_fault(fault::wire_geometry); return; }
            start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::LeadOpen);
        }
        break;
    case Phase::LeadOpen: // open/clear slug, then the length feed
        if (!any_moving()) {
            if (job_.strip_lead_um > 0) {
                // The released lead slug is swept into the waste chute and its
                // transit verified before the next feed can trap it.
                waste_next_ = WasteNext::AfterLead;
                begin_waste_sweep();
            } else {
                start_length_feed();
            }
        }
        break;
    default:
        enter_fault(fault::internal);
    }
}

void Controller::finalize_piece() {
    if (piece_done_prime_) {
        priming_ = false;
    } else if (fixture_mode_) {
        // Fixture/engineering runs never emit accepted production counts.
        ++counters_.pieces_fixture;
    } else if (!part_clear_check_enabled()) {
        // Production cannot disable handling verification (see
        // set_part_clear_check); this branch exists only for defensive
        // completeness and counts nothing as accepted.
        ++counters_.pieces_fixture;
    } else {
        // Only here does a completed piece become an accepted piece: the
        // discharge present->clear evidence chain is complete.
        ++counters_.pieces_accepted;
    }
    if (stock_restore_um_ != 0) {
        // The stock may only be restored after the takeaway confirmed clearance.
        if (!wire::restore_allowed(part_clear_check_enabled(), part_clear_confirmed_)) { enter_fault(fault::wire_geometry); return; }
        const std::int32_t restore = stock_restore_um_;
        stock_restore_um_ = 0;
        start_tracked_feed(ctrl::axis::feed_left, restore, ctrl::axis::strip_speed_mm_s, ctrl::axis::strip_accel_mm_s2);
        stock_tip_um_ += restore;
        set_phase(Phase::RestoreStock);
        return;
    }
    complete_or_launch();
}

void Controller::complete_or_launch() {
    if (!priming_ && counters_.pieces_done >= job_.quantity) {
        settings_.crc = settings_crc(settings_);
        if (!board_.store_settings(&settings_, sizeof settings_)) {
            // A completed batch must not silently lose its lifetime-count
            // record. Do not classify the already completed part as scrap.
            set_state(State::Ready);
            enter_fault(fault::internal);
            return;
        }
        beep_until_us_ = now_us_ + ms_to_us(500);
        set_phase(Phase::Idle);
        set_state(State::Ready);
        return;
    }
    launch_pass(); // next piece (or the prime feed)
}

// ----------------------------------------------------------------- commands
CommandResult Controller::load_job(const Job& job) {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    if (state_ == State::Paused) return CommandResult::WrongState;
    if (validate(job) != JobError::None) return CommandResult::InvalidArgument;
    Settings updated = settings_;
    updated.stored_job = job;
    updated.stored_job_valid = Settings::stored_job_marker;
    // A new recipe invalidates any previous commissioning approval.
    updated.approval = 0;
    updated.approval_key = 0;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    job_ = job;
    job_loaded_ = true;
    counters_ = Counters{};
    return CommandResult::Accepted;
}

CommandResult Controller::clear_job() {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    Settings updated = settings_;
    updated.stored_job = Job{};
    updated.stored_job_valid = 0;
    updated.approval = 0;
    updated.approval_key = 0;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    job_ = Job{};
    job_loaded_ = false;
    if (state_ == State::Paused) set_state(State::Ready);
    return CommandResult::Accepted;
}

CommandResult Controller::start() {
    if (state_ == State::Fault) return CommandResult::Faulted;
    if (!inputs_healthy()) return CommandResult::WrongState;
    switch (state_) {
    case State::Unhomed: begin_homing(); return CommandResult::Accepted;
    case State::Ready:
        if (!job_loaded_) { beep_until_us_ = now_us_ + ms_to_us(150); return CommandResult::NoJob; }
        // Production start requires the persistent commissioning approval.
        // Fixture mode runs engineering cycles without approval but can never
        // emit accepted counts or a persistent approval.
        if (!fixture_mode_ && !approved()) return CommandResult::NotApproved;
        begin_job_run(); return CommandResult::Accepted;
    case State::Paused:
        if (!homed_) return CommandResult::NotHomed;
        if (!job_loaded_) return CommandResult::NoJob;
        if (!fixture_mode_ && !approved()) return CommandResult::NotApproved;
        begin_job_run(); return CommandResult::Accepted; // re-primes; partial piece already counted as scrap
    default: return CommandResult::Busy;
    }
}

CommandResult Controller::stop() {
    switch (state_) {
    case State::Running: begin_controlled_stop(State::Paused); return CommandResult::Accepted;
    case State::Homing: begin_controlled_stop(State::Unhomed); return CommandResult::Accepted;
    case State::Jogging: begin_controlled_stop(homed_ ? State::Ready : State::Unhomed); return CommandResult::Accepted;
    case State::Stopping: return CommandResult::Accepted;
    default: return CommandResult::WrongState;
    }
}

CommandResult Controller::reset() {
    switch (state_) {
    case State::Fault:
        if (!inputs_healthy()) return CommandResult::WrongState;
        if (!board_.clear_latched_faults()) return CommandResult::WrongState;
        faults_ = 0;
        set_state(State::Unhomed);
        return CommandResult::Accepted;
    case State::Paused:
        set_state(State::Ready);
        return CommandResult::Accepted;
    case State::Ready:
    case State::Unhomed:
        return CommandResult::Accepted;
    default: return CommandResult::Busy;
    }
}

CommandResult Controller::home() {
    if (state_ == State::Fault) return CommandResult::Faulted;
    if (state_ != State::Unhomed && state_ != State::Ready) return CommandResult::Busy;
    if (!inputs_healthy()) return CommandResult::WrongState;
    begin_homing();
    return CommandResult::Accepted;
}

CommandResult Controller::jog(Axis a, bool forward, std::int32_t distance_um, std::int32_t speed_um_s) {
    if (state_ == State::Fault) return CommandResult::Faulted;
    if (state_ != State::Unhomed && state_ != State::Ready) return CommandResult::Busy;
    if (!inputs_healthy()) return CommandResult::WrongState;
    if (static_cast<std::uint8_t>(a) >= ctrl::axis::count || distance_um <= 0 || speed_um_s <= 0) return CommandResult::InvalidArgument;
    const bool blade = a == ctrl::axis::blade_front || a == ctrl::axis::blade_rear;
    if (blade) {
        if (!homed_) return CommandResult::NotHomed;
        // F33: widened checked arithmetic before the range decision; no signed
        // overflow for any distance/position combination.
        const std::int64_t target = static_cast<std::int64_t>(blade_travel_um(a)) +
                                    (forward ? static_cast<std::int64_t>(distance_um) : -static_cast<std::int64_t>(distance_um));
        if (target < 0 || target > static_cast<std::int64_t>(ctrl::axis::blade_limit_mm * 1000.0)) return CommandResult::InvalidArgument;
    } else {
        if (distance_um > 1000000) return CommandResult::InvalidArgument;
    }
    jog_axis_ = a;
    jog_forward_ = forward;
    jog_distance_um_ = distance_um;
    jog_speed_um_s_ = speed_um_s;
    commissioning_ = false;
    out_.drives_disabled = false;
    drives_enabled_at_us_ = now_us_;
    set_state(State::Jogging);
    set_phase(Phase::EnableSettle);
    return CommandResult::Accepted;
}

CommandResult Controller::commission_axis(Axis a, bool logical_forward) {
    if (static_cast<std::uint8_t>(a) >= ctrl::axis::count) return CommandResult::InvalidArgument;
    if (state_ == State::Fault) return CommandResult::Faulted;
    if (state_ != State::Unhomed && state_ != State::Ready) return CommandResult::Busy;
    if (!inputs_healthy()) return CommandResult::WrongState;
    const bool blade = a == ctrl::axis::blade_front || a == ctrl::axis::blade_rear;
    if (blade) {
        if (!homed_) return CommandResult::NotHomed;
        if (!logical_forward) return CommandResult::InvalidArgument; // validate by leaving the open-end sensor
        const std::int64_t current = blade_travel_um(a);
        if (current + commission_blade_move_um > static_cast<std::int64_t>(ctrl::axis::blade_limit_mm * 1000.0))
            return CommandResult::InvalidArgument;
        const bool sensor = a == ctrl::axis::blade_front ? in_.home_front : in_.home_rear;
        if (!sensor) return CommandResult::InvalidArgument; // missing feedback: not on the open-end sensor
        commission_sensor_start_ = true;
        commission_encoder_start_ = 0;
    } else {
        const std::uint8_t side = a == ctrl::axis::feed_right ? 1 : 0;
        if (!board_.encoder_healthy(side)) return CommandResult::WrongState;
        commission_encoder_start_ = board_.encoder_count(side);
        commission_sensor_start_ = false;
    }
    commission_axis_ = a;
    commission_forward_ = logical_forward;
    commission_result_ = CommandResult::Unsupported; // pending until the move completes
    commissioning_ = true;
    jog_axis_ = a;
    jog_forward_ = logical_forward;
    jog_distance_um_ = static_cast<std::int32_t>(blade ? commission_blade_move_um : commission_feed_move_um);
    jog_speed_um_s_ = static_cast<std::int32_t>(ctrl::axis::strip_speed_mm_s * 1000.0);
    out_.drives_disabled = false;
    drives_enabled_at_us_ = now_us_;
    set_state(State::Jogging);
    set_phase(Phase::EnableSettle);
    return CommandResult::Accepted;
}

void Controller::finish_commissioning() {
    const bool blade = commission_axis_ == ctrl::axis::blade_front || commission_axis_ == ctrl::axis::blade_rear;
    Settings updated = settings_;
    CommandResult result = CommandResult::Accepted;
    if (blade) {
        const bool sensor_now = commission_axis_ == ctrl::axis::blade_front ? in_.home_front : in_.home_rear;
        if (!commission_sensor_start_) {
            result = CommandResult::InvalidArgument; // missing feedback
        } else if (sensor_now) {
            // Still on the sensor after moving toward the cut: the axis ran the
            // wrong way, so store the inverted validated polarity.
            updated.axis_sign[commission_axis_] = static_cast<std::int8_t>(-updated.axis_sign[commission_axis_]);
        }
        homed_ = false; // the commissioning move left the home reference
    } else {
        const std::uint8_t side = commission_axis_ == ctrl::axis::feed_right ? 1 : 0;
        const std::int64_t raw_delta = board_.encoder_count(side) - commission_encoder_start_;
        const std::int64_t logical_delta = updated.wheel_sign[side] < 0 ? -raw_delta : raw_delta;
        const std::int64_t magnitude = logical_delta < 0 ? -logical_delta : logical_delta;
        if (magnitude < commission_min_counts) {
            result = CommandResult::InvalidArgument; // missing feedback: nothing measurable moved
        } else if ((logical_delta > 0) != commission_forward_) {
            updated.axis_sign[commission_axis_] = static_cast<std::int8_t>(-updated.axis_sign[commission_axis_]);
        }
    }
    if (result == CommandResult::Accepted) {
        updated.crc = settings_crc(updated);
        if (board_.store_settings(&updated, sizeof updated)) settings_ = updated;
        else result = CommandResult::StorageError;
    }
    commission_result_ = result;
    commissioning_ = false;
    set_phase(Phase::Idle);
    set_state(homed_ ? State::Ready : State::Unhomed);
}

CommandResult Controller::set_axis_sign(Axis a, std::int8_t sign) {
    if (static_cast<std::uint8_t>(a) >= ctrl::axis::count || (sign != 1 && sign != -1)) return CommandResult::InvalidArgument;
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    Settings updated = settings_;
    updated.axis_sign[a] = sign;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

CommandResult Controller::set_wheel_sign(std::uint8_t side, std::int8_t sign) {
    if (side > 1 || (sign != 1 && sign != -1)) return CommandResult::InvalidArgument;
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    Settings updated = settings_;
    updated.wheel_sign[side] = sign;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

CommandResult Controller::set_calibration(std::int32_t w0_counts_per_metre, std::int32_t w1_counts_per_metre,
                                          std::int32_t piece_counts_per_metre) {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    // ifm E60137 wheel 200 mm nominal circumference, 8192 counts/rev (2048 PPR x4): 40960 nominal; allow +/-5 % for wear/preload.
    const std::int32_t lo = ctrl::enc::nominal_counts_per_metre * 95 / 100, hi = ctrl::enc::nominal_counts_per_metre * 105 / 100;
    if (w0_counts_per_metre < lo || w0_counts_per_metre > hi ||
        w1_counts_per_metre < lo || w1_counts_per_metre > hi ||
        piece_counts_per_metre < lo || piece_counts_per_metre > hi) return CommandResult::InvalidArgument;
    Settings updated = settings_;
    updated.counts_per_metre[0] = w0_counts_per_metre;
    updated.counts_per_metre[1] = w1_counts_per_metre;
    updated.counts_per_metre[2] = piece_counts_per_metre;
    // Calibration is part of the approval identity: invalidate the old record.
    updated.approval = 0;
    updated.approval_key = 0;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

CommandResult Controller::approve() {
    if (state_ == State::Fault) return CommandResult::Faulted;
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    if (!inputs_healthy()) return CommandResult::WrongState;
    // Fixture/engineering mode must not emit a persistent approval equivalent
    // to a qualified production build.
    if (fixture_mode_) return CommandResult::Unsupported;
    if (!job_loaded_ || validate(job_) != JobError::None) return CommandResult::NoJob;
    // Production approval requires handling verification to be active.
    if (!part_clear_check_enabled()) return CommandResult::Unsupported;
    Settings updated = settings_;
    updated.stored_job = job_;
    updated.stored_job_valid = Settings::stored_job_marker;
    updated.approval_key = commissioning_identity(updated);
    updated.approval = Settings::approval_marker;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

CommandResult Controller::set_part_clear_check(bool enabled) {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    // Production must not bypass required handling verification. Disabling the
    // check is only permitted in explicit service fixture mode.
    if (!enabled && !fixture_mode_) return CommandResult::Unsupported;
    Settings updated = settings_;
    updated.part_clear_check = enabled ? 1 : 0;
    updated.approval = 0; // a production-setting change requires re-approval
    updated.approval_key = 0;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

CommandResult Controller::set_fixture_mode(bool enabled) {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    fixture_mode_ = enabled;
    // Entering or leaving fixture mode returns handling verification to the
    // production default and invalidates any approval created under different
    // settings; fixture state is never persisted across a reboot.
    Settings updated = settings_;
    updated.part_clear_check = 1;
    updated.approval = 0;
    updated.approval_key = 0;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

Telemetry Controller::telemetry() const {
    Telemetry t{};
    t.state = state_; t.phase = phase_; t.faults = faults_;
    t.inputs = in_; t.outputs = out_;
    t.job_loaded = job_loaded_; t.approved = approved(); t.job = job_; t.counters = counters_;
    t.blade_um[0] = blade_travel_um(ctrl::axis::blade_front);
    t.blade_um[1] = blade_travel_um(ctrl::axis::blade_rear);
    t.feed_um[0] = feed_steps_to_um(axis_[ctrl::axis::feed_left].position_steps);
    t.feed_um[1] = feed_steps_to_um(axis_[ctrl::axis::feed_right].position_steps);
    t.encoder_counts[0] = logical_encoder_counts(0);
    t.encoder_counts[1] = logical_encoder_counts(1);
    t.piece_encoder_counts = logical_piece_counts();
    t.stock_tip_um = stock_tip_um_;
    t.piece_tail_um = piece_tail_um_;
    t.piece_present = piece_present_;
    t.fixture_mode = fixture_mode_;
    t.settings = settings_;
    t.uptime_us = now_us_;
    return t;
}

} // namespace starz::fw
