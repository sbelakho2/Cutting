#include "controller.hpp"
#include "crc16.hpp"
#include <cmath>
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
constexpr std::uint16_t alm_bit(Axis a) { return static_cast<std::uint16_t>(1u << static_cast<unsigned>(a)); }

// Deliberately excludes the final CRC word. The Settings layout assertion
// guarantees this covers the exact persisted 44-byte payload.
std::uint32_t settings_crc(const Settings& s) {
    return crc16(reinterpret_cast<const std::uint8_t*>(&s), sizeof(Settings) - sizeof(std::uint32_t));
}
} // namespace

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
        // 90 deg V blades: apex-to-axis distance a = sqrt(2) * r at the cut radius r.
        const double r_mm = (j.wire_diameter_um / 2.0 - j.strip_depth_um) / 1000.0;
        const double apex_mm = std::sqrt(2.0) * r_mm;
        const double travel_mm = (ctrl::axis::blade_cut_mm - 0.2) - apex_mm; // 0.2 mm apex overlap at sever
        if (travel_mm < 0.0 || travel_mm > ctrl::axis::blade_cut_mm) return JobError::Geometry;
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
        loaded.crc == settings_crc(loaded) && loaded.counts_per_metre[0] > 0 && loaded.counts_per_metre[1] > 0) {
        settings_ = loaded;
        if (settings_.stored_job_valid == Settings::stored_job_marker && validate(settings_.stored_job) == JobError::None) {
            job_ = settings_.stored_job;
            job_loaded_ = true;
        }
    } else {
        settings_ = Settings{};
        settings_.crc = settings_crc(settings_);
    }
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
    const double r_mm = (job_.wire_diameter_um / 2.0 - job_.strip_depth_um) / 1000.0;
    const double travel_mm = (ctrl::axis::blade_cut_mm - 0.2) - std::sqrt(2.0) * r_mm;
    return static_cast<std::int32_t>(std::llround(travel_mm * 1000.0));
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
        if (all_idle()) { axis_[0].moving = axis_[1].moving = axis_[2].moving = axis_[3].moving = false; set_phase(Phase::Idle); set_state(homed_ ? State::Ready : State::Unhomed); }
        else if (phase_timed_out(ctrl::timing::move_timeout_ms)) enter_fault(fault::move_timeout);
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
    in_ = raw_;
    in_.start = start_.stable && start_.level;
    in_.stop_ok = stop_.stable && stop_.level;
    in_.reset = reset_.stable && reset_.level;
    in_.safety_ok = safety_.stable && safety_.level;
    in_.home_front = home_front_.stable && home_front_.level;
    in_.home_rear = home_rear_.stable && home_rear_.level;
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
    const std::uint16_t board_faults = board_.latched_faults();
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

void Controller::enter_fault(std::uint16_t bits) {
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
    board_.set_direction(a, spec.forward);
    ax.moving = true;
    ax.stall_fault = stall_fault;
    ax.queue_limit = max_run == 1 ? 1 : Board::step_fifo_depth;
    ax.move_start_us = now_us_;
    ax.encoder_start = board_.encoder_count(a == ctrl::axis::feed_right ? 1 : 0);
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
            const std::int64_t enc = board_.encoder_count(a == ctrl::axis::feed_right ? 1 : 0) - ax.encoder_start;
            const std::int32_t moved_um = std::abs(measured_um(a == ctrl::axis::feed_right ? 1 : 0, enc));
            if (issued_um >= 5000 && moved_um < 1000) { enter_fault(ax.stall_fault); return; }
        }
        if (ax.moving && now_us_ - ax.move_start_us > ms_to_us(ctrl::timing::move_timeout_ms)) { enter_fault(fault::move_timeout); return; }
    }
}

// ------------------------------------------------------------------- homing
void Controller::begin_homing() {
    for (auto& a : axis_) a.moving = false;
    home_sub_done_[0] = home_sub_done_[1] = false;
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
    cycle_started_ = false;
    correction_done_ = false;
    out_.drives_disabled = false;
    drives_enabled_at_us_ = now_us_;
    set_state(State::Running);
    set_phase(Phase::EnableSettle);
}

bool Controller::begin_jog_move() {
    const bool blade = jog_axis_ == ctrl::axis::blade_front || jog_axis_ == ctrl::axis::blade_rear;
    if (blade) {
        const std::int32_t target = blade_travel_um(jog_axis_) + (jog_forward_ ? jog_distance_um_ : -jog_distance_um_);
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
    const bool strips = job_.strip_lead_um != 0 || job_.strip_tail_um != 0;
    service_axes();
    if (state_ != State::Running) return;

    switch (phase_) {
    case Phase::EnableSettle:
        if (now_us_ - drives_enabled_at_us_ >= ms_to_us(timing::drive_enable_settle_ms)) {
            if (blade_travel_um(axis::blade_front) != 0 || blade_travel_um(axis::blade_rear) != 0) {
                start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
                set_phase(Phase::Open);
            } else set_phase(Phase::PieceDone); // PieceDone launches the next Feed
        }
        break;
    case Phase::Feed:
        if (!any_moving()) {
            if (priming_) { set_phase(Phase::Cut); start_blade_moves_um(static_cast<std::int32_t>(axis::blade_cut_mm * 1000), axis::blade_speed_mm_s, axis::blade_accel_mm_s2); }
            else set_phase(Phase::FeedVerify);
        }
        break;
    case Phase::FeedVerify: {
        const std::int32_t residual[2] = {
            feed_target_um_ - measured_um(0, board_.encoder_count(0) - feed_encoder_start_[0]),
            feed_target_um_ - measured_um(1, board_.encoder_count(1) - feed_encoder_start_[1]),
        };
        counters_.last_residual_um = residual[0];
        for (std::int32_t r : residual)
            if (std::abs(r) > std::abs(counters_.worst_residual_um)) counters_.worst_residual_um = r;
        const std::int32_t deadband = static_cast<std::int32_t>(2.0 * enc::nominal_um_per_count);
        if (std::abs(residual[0]) > enc::max_slip_um || std::abs(residual[1]) > enc::max_slip_um ||
            std::abs(residual[0] - residual[1]) > deadband) {
            enter_fault(fault::length_slip);
            return;
        }
        const std::int32_t correction = static_cast<std::int32_t>(
            std::llround((static_cast<double>(residual[0]) + residual[1]) / 2.0));
        if (!correction_done_ && std::abs(correction) > deadband) {
            correction_done_ = true;
            start_feed_move_um(L, correction, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            start_feed_move_um(R, correction, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            set_phase(Phase::FeedCorrect);
        } else {
            start_blade_moves_um(static_cast<std::int32_t>(axis::blade_cut_mm * 1000), axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::Cut);
        }
        break;
    }
    case Phase::FeedCorrect:
        if (!any_moving()) set_phase(Phase::FeedVerify);
        break;
    case Phase::Cut:
        if (!any_moving()) {
            start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::OpenAfterCut);
        }
        break;
    case Phase::OpenAfterCut:
        if (!any_moving()) {
            if (!strips) { set_phase(Phase::Eject); if (!priming_) start_feed_move_um(R, job::ejection_um, axis::feed_speed_mm_s, axis::feed_accel_mm_s2); }
            else {
                if (job_.strip_lead_um) start_feed_move_um(L, job_.strip_lead_um, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
                if (job_.strip_tail_um && !priming_) start_feed_move_um(R, -job_.strip_tail_um, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
                set_phase(Phase::StripPosition);
            }
        }
        break;
    case Phase::StripPosition:
        if (!any_moving()) {
            start_blade_moves_um(strip_travel_um(), axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::StripClose);
        }
        break;
    case Phase::StripClose:
        if (!any_moving()) {
            if (job_.strip_lead_um) start_feed_move_um(L, -job_.strip_lead_um, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            if (job_.strip_tail_um && !priming_) start_feed_move_um(R, job_.strip_tail_um, axis::strip_speed_mm_s, axis::strip_accel_mm_s2);
            set_phase(Phase::StripPull);
        }
        break;
    case Phase::StripPull:
        if (!any_moving()) {
            start_blade_moves_um(0, axis::blade_speed_mm_s, axis::blade_accel_mm_s2);
            set_phase(Phase::Open);
        }
        break;
    case Phase::Open:
        if (!any_moving()) {
            if (priming_) set_phase(Phase::PieceDone);
            else { start_feed_move_um(R, job::ejection_um, axis::feed_speed_mm_s, axis::feed_accel_mm_s2); set_phase(Phase::Eject); }
        }
        break;
    case Phase::Eject:
        if (!any_moving()) set_phase(Phase::PieceDone);
        break;
    case Phase::PieceDone:
        if (cycle_started_) {
            if (priming_) priming_ = false;
            else {
                ++counters_.pieces_done;
                ++settings_.pieces_total;
                if (counters_.pieces_done >= job_.quantity) {
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
                    break;
                }
            }
        }
        cycle_started_ = true;
        // Next piece (or the prime feed).
        correction_done_ = false;
        feed_encoder_start_[0] = board_.encoder_count(0);
        feed_encoder_start_[1] = board_.encoder_count(1);
        if (priming_) {
            feed_target_um_ = spec::stock_clear_um;
            start_feed_move_um(L, feed_target_um_, axis::feed_speed_mm_s, axis::feed_accel_mm_s2, fault::no_wire);
            start_feed_move_um(R, feed_target_um_, axis::feed_speed_mm_s, axis::feed_accel_mm_s2);
        } else {
            feed_target_um_ = job_.length_um;
            start_feed_move_um(L, feed_target_um_, axis::feed_speed_mm_s, axis::feed_accel_mm_s2, fault::encoder_stalled);
            start_feed_move_um(R, feed_target_um_, axis::feed_speed_mm_s, axis::feed_accel_mm_s2);
        }
        set_phase(Phase::Feed);
        break;
    default:
        enter_fault(fault::internal);
    }
}

// ----------------------------------------------------------------- commands
CommandResult Controller::load_job(const Job& job) {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    if (state_ == State::Paused) return CommandResult::WrongState;
    if (validate(job) != JobError::None) return CommandResult::InvalidArgument;
    Settings updated = settings_;
    updated.stored_job = job;
    updated.stored_job_valid = Settings::stored_job_marker;
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
        begin_job_run(); return CommandResult::Accepted;
    case State::Paused:
        if (!homed_) return CommandResult::NotHomed;
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
    if (a >= ctrl::axis::count || distance_um <= 0 || speed_um_s <= 0) return CommandResult::InvalidArgument;
    const bool blade = a == ctrl::axis::blade_front || a == ctrl::axis::blade_rear;
    if (blade) {
        if (!homed_) return CommandResult::NotHomed;
        const std::int32_t target = blade_travel_um(a) + (forward ? distance_um : -distance_um);
        if (target < 0 || target > static_cast<std::int32_t>(ctrl::axis::blade_limit_mm * 1000.0)) return CommandResult::InvalidArgument;
    } else {
        if (distance_um > 1000000) return CommandResult::InvalidArgument;
    }
    jog_axis_ = a;
    jog_forward_ = forward;
    jog_distance_um_ = distance_um;
    jog_speed_um_s_ = speed_um_s;
    out_.drives_disabled = false;
    drives_enabled_at_us_ = now_us_;
    set_state(State::Jogging);
    set_phase(Phase::EnableSettle);
    return CommandResult::Accepted;
}

CommandResult Controller::set_calibration(std::int32_t left_counts_per_metre, std::int32_t right_counts_per_metre) {
    if (state_ == State::Running || state_ == State::Stopping || state_ == State::Homing || state_ == State::Jogging) return CommandResult::Busy;
    // ifm E60137 wheel 200 mm nominal circumference, 4096 counts/rev: 20480 nominal; allow +/-5 % for wear/preload.
    const std::int32_t lo = ctrl::enc::nominal_counts_per_metre * 95 / 100, hi = ctrl::enc::nominal_counts_per_metre * 105 / 100;
    if (left_counts_per_metre < lo || left_counts_per_metre > hi || right_counts_per_metre < lo || right_counts_per_metre > hi) return CommandResult::InvalidArgument;
    Settings updated = settings_;
    updated.counts_per_metre[0] = left_counts_per_metre;
    updated.counts_per_metre[1] = right_counts_per_metre;
    updated.crc = settings_crc(updated);
    if (!board_.store_settings(&updated, sizeof updated)) return CommandResult::StorageError;
    settings_ = updated;
    return CommandResult::Accepted;
}

Telemetry Controller::telemetry() const {
    Telemetry t{};
    t.state = state_; t.phase = phase_; t.faults = faults_;
    t.inputs = in_; t.outputs = out_;
    t.job_loaded = job_loaded_; t.job = job_; t.counters = counters_;
    t.blade_um[0] = blade_travel_um(ctrl::axis::blade_front);
    t.blade_um[1] = blade_travel_um(ctrl::axis::blade_rear);
    t.feed_um[0] = feed_steps_to_um(axis_[ctrl::axis::feed_left].position_steps);
    t.feed_um[1] = feed_steps_to_um(axis_[ctrl::axis::feed_right].position_steps);
    t.encoder_counts[0] = board_.encoder_count(0);
    t.encoder_counts[1] = board_.encoder_count(1);
    t.settings = settings_;
    t.uptime_us = now_us_;
    return t;
}

} // namespace starz::fw
