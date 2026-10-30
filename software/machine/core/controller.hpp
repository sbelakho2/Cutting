#pragma once
// Machine controller: operator buttons, safety supervision, blade homing,
// cut/strip cycle sequencing, encoder-verified feed length and host commands.
// Runs at ctrl::mcu::control_period_us on the target; the host tests drive
// tick() with a simulated Board.
//
// NOTES (physical I/O and observability contract, ctrl::sensing):
// - The two stock-side measuring wheels (Board::encoder_count 0/1) ride
//   continuous stock and can NEVER observe a severed piece. Stock moves are
//   verified against them; piece moves are verified against the downstream
//   station Board::piece_encoder_count() only while the piece occupies that
//   station (M27 occupancy windows and origin transfer). The unobserved
//   ejection remainder is covered by the discharge present->clear evidence or
//   the piece is quarantined - never accepted on commanded steps alone.
// - Inputs::part_clear (true = clear) is the discharge-clearance channel. In
//   production (part_clear_check = 1, the default) a finished piece is counted
//   accepted only after a debounced present->clear transition captured in the
//   physical observation window that begins at ejection. An unassigned pin
//   must read false so absence of hardware cannot fake acceptance.
// - Inputs::waste_clear (true = clear) with Outputs::waste_sweep forms the
//   blade-region waste path for the 10 mm prime stub and 3-25 mm strip slugs:
//   every released body must produce a present->clear transit after the sweep
//   before the next feed or blade close. A body the right cassette cannot
//   reach (shorter than ctrl::sensing::piece_body_grip_x_mm) is never ejected
//   by feed motion.
// - Production START (ready with a retained recipe, or paused resume) requires
//   a persistent commissioning approval (Settings::approval) keyed by
//   commissioning_identity() to the calibration, the retained recipe and the
//   firmware revision. Calibration/recipe changes clear it; approve it over
//   the host link (Msg::Approve) after commissioning.
// - Service fixture mode (set_fixture_mode, RAM-only, never persisted) permits
//   engineering runs with handling verification disabled. Fixture runs must
//   not emit accepted production counts (they count to pieces_fixture), cannot
//   create persistent approval, and are visibly distinct in telemetry.
#include <cstdint>
#include "types.hpp"
#include "board.hpp"
#include "profile.hpp"

namespace starz::fw {

// F08 process geometry: the ideal per-jaw strip closure is
// blade_open_apex/2 - sqrt(2)*r. The sever travel stays at the CAD
// blade_cut_travel, whose explicit per-jaw overlap beyond the closed-jaw
// position is the constant below (0.2 mm total apex overlap at sever).
inline constexpr std::int32_t blade_sever_overlap_um =
    static_cast<std::int32_t>(ctrl::axis::blade_cut_mm * 1000.0) -
    static_cast<std::int32_t>(ctrl::axis::blade_open_mm * 1000.0 / 2.0);
static_assert(blade_sever_overlap_um == 100, "sever overlap must stay the explicit 0.1 mm/jaw CAD allowance");

// F07 wire-body model. The cycle sequencer validates every planned move and
// score against these predicates and rejects (fault::wire_geometry) anything
// that would overlap the stock and the severed piece, score the wrong end,
// lose the retained nip grip, or restore the stock before clearance. Positions
// are logical X micrometers relative to the cut plane.
namespace wire {
inline constexpr std::int32_t separation_um = 10000;                          // stock tip vs piece tail at the tail score
inline constexpr std::int32_t grip_um = static_cast<std::int32_t>(feed::design::minimum_insulated_grip_margin * 1000.0);
// Worst-case far nip from the docs/01 §4 shortest-part arithmetic: near nip
// 75.5 + maximum centre distance 64.5 = 140.0 mm; >= 10 mm insulated material
// must remain beyond it (L - s1 - s2 >= far_nip + grip).
inline constexpr std::int32_t far_nip_um =
    static_cast<std::int32_t>((feed::design::near_nip_x + feed::design::max_centres) * 1000.0);
static_assert(far_nip_um == 140000 && grip_um == 10000, "nip/grip geometry constants drifted");
// stock_tip <= piece_tail for any present piece: no interpenetration.
inline bool bodies_valid(std::int32_t stock_tip_um, std::int32_t piece_tail_um, bool piece_present) noexcept {
    return !piece_present || stock_tip_um <= piece_tail_um;
}
// A lead score requires the stock tip advanced by exactly s1 with no piece.
inline bool lead_score_valid(std::int32_t stock_tip_um, std::int32_t lead_um, bool piece_present) noexcept {
    return !piece_present && lead_um > 0 && stock_tip_um == lead_um;
}
// A tail score requires the piece tail at -s2, the stock tip a separation away
// (< the piece tail) and the retained insulated length past the far nip.
inline bool tail_score_valid(std::int32_t stock_tip_um, std::int32_t piece_tail_um, std::int32_t piece_len_um,
                             std::int32_t lead_um, std::int32_t tail_um, bool piece_present) noexcept {
    if (!piece_present || tail_um <= 0 || piece_tail_um != -tail_um) return false;
    if (piece_tail_um - stock_tip_um < separation_um) return false;
    return static_cast<std::int64_t>(piece_len_um) - lead_um - tail_um >= far_nip_um + grip_um;
}
// The stock is restored only after the takeaway confirmed clearance (when the
// optional check is enabled).
inline bool restore_allowed(bool clear_check_enabled, bool clear_confirmed) noexcept {
    return !clear_check_enabled || clear_confirmed;
}
} // namespace wire

enum class CommandResult : std::uint8_t {
    Accepted = 0, InvalidArgument = 1, WrongState = 2, Busy = 3, Faulted = 4,
    NotHomed = 5, NoJob = 6, Unsupported = 7, MalformedFrame = 8, StorageError = 9,
    NotApproved = 10
};

enum class Phase : std::uint8_t {
    Idle = 0,
    // homing (both blade axes)
    HomeLeave, HomeSeek, HomeBackoff, HomeApproach,
    // cycle; the first pass after START is the prime (feed stock clearance,
    // sever the trimmed stub, discharge) and does not count a piece.
    EnableSettle, Open, Feed, FeedVerify, FeedCorrect, Cut, OpenAfterCut,
    // Lead strip (docs/01 §4 step 1): advance stock s1, score at the cut plane,
    // retract s1+3 with the blades retaining the slug, open and clear it.
    LeadAdvance, LeadScore, LeadRetract, LeadOpen,
    // Tail strip (docs/01 §4 step 4): withdraw stock s2+10, backfeed the piece
    // s2 (10 mm separation), score at the cut plane, pull the piece s2+3.
    TailWithdraw, TailScore, TailPull, TailOpen,
    Eject, AwaitClear, RestoreStock, PieceDone,
    Jog,
    Stop,
    // Waste handling (M30): a released prime stub or insulation slug is swept
    // into the waste chute and verified by a present->clear transit on the
    // waste sensor before any further feed or blade close can trap it.
    // Appended after Stop so existing wire values stay stable.
    WasteSweep, WasteClear
};

struct Counters {
    std::uint32_t pieces_done = 0;      // PieceDone events in the current job (physical completions).
    std::uint32_t pieces_accepted = 0;  // production-good pieces (handling evidence complete).
    std::uint32_t pieces_scrap = 0;     // partial pieces discarded after a stop/fault.
    std::uint32_t pieces_fixture = 0;   // fixture-mode completions; never accepted production output.
    std::uint32_t cut_cycles = 0;       // wear-producing sever events (prime + production + scrap cuts).
    std::int32_t last_residual_um = 0;  // target minus measured on the last verified feed
    std::int32_t worst_residual_um = 0;
};

struct Telemetry {
    State state;
    Phase phase;
    FaultBits faults;
    Inputs inputs;
    Outputs outputs;
    bool job_loaded;
    bool approved;
    Job job;
    Counters counters;
    std::int32_t blade_um[2];       // travel from open, commanded
    std::int64_t feed_um[2];        // cumulative commanded transport, left/right
    std::int64_t encoder_counts[2]; // stock-side wheels W0/W1, commissioned polarity applied
    std::int64_t piece_encoder_counts; // downstream station D, commissioned polarity applied
    // Explicit wire-body model positions, relative to the cut plane X0.
    std::int32_t stock_tip_um;      // stock leading end
    std::int32_t piece_tail_um;     // severed piece tail (valid while piece_present)
    bool piece_present;
    bool fixture_mode;              // service fixture mode: no accepted counts, no approval
    Settings settings;
    std::uint64_t uptime_us;
};

class Controller {
public:
    explicit Controller(Board& board);

    void init();                      // call once after the board is ready
    void tick();                      // call every ctrl::mcu::control_period_us
    void pump();                      // call as often as possible between ticks: refills step FIFOs only

    // Host commands (same effect as the panel where a button exists).
    CommandResult load_job(const Job& job);
    CommandResult clear_job();
    CommandResult start();
    CommandResult stop();
    CommandResult reset();
    CommandResult home();
    CommandResult jog(Axis axis, bool forward, std::int32_t distance_um, std::int32_t speed_um_s);
    CommandResult set_calibration(std::int32_t w0_counts_per_metre, std::int32_t w1_counts_per_metre,
                                  std::int32_t piece_counts_per_metre);
    // Commissioning/recipe approval (F12 interlock); invalidated by any
    // calibration, recipe or firmware-identity change.
    CommandResult approve();
    bool approved() const noexcept;
    // Discharge-clearance/handling verification configuration (F11). Production
    // requires it (default 1). Disabling is only permitted in explicit service
    // fixture mode and invalidates approval; fixture runs never count accepted.
    CommandResult set_part_clear_check(bool enabled);
    // Service fixture mode (RAM-only, cleared at init): engineering runs with
    // handling verification optional, no persistent approval, fixture-only
    // counts. Production behaviour is the default.
    CommandResult set_fixture_mode(bool enabled);
    bool fixture_mode() const noexcept { return fixture_mode_; }
    // Validated direction polarity commissioning (F33). Runs a short move on
    // the axis and stores the sign that makes logical command and feedback
    // agree. Returns Accepted when the move has been started; the measured
    // outcome is then available from commission_result().
    CommandResult commission_axis(Axis axis, bool logical_forward);
    CommandResult commission_result() const noexcept { return commission_result_; }
    CommandResult set_axis_sign(Axis axis, std::int8_t sign);
    CommandResult set_wheel_sign(std::uint8_t side, std::int8_t sign);

    Telemetry telemetry() const;
    State state() const noexcept { return state_; }
    Phase phase() const noexcept { return phase_; }
    FaultBits faults() const noexcept { return faults_; }
    const Counters& counters() const noexcept { return counters_; }
    // Ideal per-jaw strip closure for the loaded recipe (F08).
    std::int32_t strip_travel_um() const;
    // Distance of a piece move [tail0, tail0+delta] that downstream station D
    // physically observes (occupancy tail <= D <= tail+len throughout that part
    // of the move). Counts outside occupancy are zero and never fabricated.
    // Exposed for the observability tests (M27).
    static std::int32_t piece_station_observed_um(std::int32_t tail0_um, std::int32_t len_um, std::int32_t delta_um);

private:
    struct AxisState {
        StepStreamer streamer;
        std::int64_t position_steps = 0; // commanded, signed
        bool moving = false;
        std::uint16_t stall_fault = 0;   // 0 = no encoder stall supervision for this move
        std::int64_t encoder_start = 0;
        std::uint32_t queue_limit = Board::step_fifo_depth;
        std::uint64_t move_start_us = 0;
    };
    struct Debounced {
        bool level = false;
        bool stable = false;
        std::uint64_t since_us = 0;
        std::uint64_t pressed_at_us = 0; // for hold detection
        bool consumed = false;
        void update(bool raw, std::uint64_t now, std::uint32_t debounce_us);
    };
    // One commanded feed move with its physical observability plan (M27).
    // station: 0/1 = stock wheels W0/W1 (stock body), 2 = downstream station D
    // (piece body). observed_expect_um is the part of the commanded move during
    // which the observing station is physically occupied by the moved body;
    // the remainder is unobserved transport and is legal only for the final
    // ejection, where discharge evidence covers it before acceptance.
    struct TrackedFeed {
        std::int32_t expect_um = 0;
        std::int32_t observed_expect_um = 0;
        std::int64_t encoder_start = 0;
        bool active = false;
        bool body_is_piece = false;
        bool final_eject = false;
        bool strip_move = false;   // strip/pull motions use the flat strip budget
    };

    // supervision
    void sample_inputs();
    void supervise_safety();
    void enter_fault(FaultBits bits);
    void set_state(State s);
    bool inputs_healthy() const;
    void handle_buttons();
    void update_indicators();

    // motion primitives
    bool start_move(Axis a, std::int64_t delta_steps, double speed_steps_s, double accel_steps_s2,
                    std::uint16_t stall_fault = 0, std::uint32_t max_run = max_run_pulses);
    bool start_blade_moves_um(std::int32_t target_travel_um, double speed_mm_s, double accel_mm_s2,
                              std::uint32_t max_run = max_run_pulses);
    bool start_feed_move_um(Axis a, std::int32_t delta_um, double speed_mm_s, double accel_mm_s2, std::uint16_t stall_fault = 0);
    void service_axes();
    void abort_all();
    void begin_controlled_stop(State return_state);
    bool all_idle() const;
    bool any_moving() const;
    void set_phase(Phase p);
    bool phase_timed_out(std::uint32_t timeout_ms) const;

    // sequences
    void run_homing();
    void run_cycle();
    void begin_homing();
    void begin_job_run();
    bool begin_jog_move();
    std::int32_t measured_um(std::uint8_t side, std::int64_t counts_delta) const;
    std::int32_t blade_travel_um(Axis a) const;

    // strip/cycle model helpers (F07/F09/F33)
    // Internal guarded budgets are strictly SMALLER than the external
    // tolerances (handoff 12.1): half-width allocations, so correction
    // residuals cannot consume the whole external allowance.
    std::int32_t length_budget_um(std::int32_t target_um) const;   // 100 + 0.025 % of |target|
    std::int32_t strip_budget_um() const;                          // flat 100 um; strip moves
    // never inherit a length-proportional allowance (handoff 12.1).
    std::int32_t external_tolerance_um(std::int32_t target_um) const; // 200 + 0.05 % of |target|
    std::int64_t logical_encoder_counts(std::uint8_t side) const;
    std::int64_t logical_piece_counts() const;
    bool wire_state_valid() const;
    // Tracked feed with its physical observability plan. The moved body is the
    // severed piece only when the right cassette drives with a piece present
    // (stock otherwise). final_eject marks the single move whose unobserved
    // remainder must be covered by discharge evidence before acceptance.
    void start_tracked_feed(Axis a, std::int32_t delta_um, double speed_mm_s, double accel_mm_s2,
                            std::uint16_t stall_fault = 0, bool final_eject = false, bool strip_move = false);
    bool verify_tracked_feed(std::uint8_t side, bool gross_only); // stock wheels W0/W1
    bool verify_tracked_piece(bool gross_only);                   // downstream station D
    // Independent per-piece length cross-check (review F04): station D is a
    // separate instrument from the stock wheels, so the signed material it sees
    // over one piece's lifetime (length feed -> ejection) must equal the
    // commanded length. This detects common-mode stock-wheel scale error that
    // agreeing W0/W1 can never prove. D's own calibration uncertainty gates its
    // usefulness (P13); it is one more independent channel, not absolute truth.
    bool verify_piece_length_d();
    // Discharge present->clear evidence inside the physical observation window.
    bool discharge_transition_valid() const;
    bool waste_transition_valid() const;
    void arm_discharge_window();
    void begin_waste_sweep();
    void finish_waste();
    void launch_pass();
    void start_length_feed();
    void finalize_piece();
    void complete_or_launch();
    void finish_commissioning();
    bool part_clear_check_enabled() const { return settings_.part_clear_check != 0; }

    Board& board_;
    State state_ = State::PowerOn;
    Phase phase_ = Phase::Idle;
    FaultBits faults_ = 0;
    Inputs raw_{};
    Inputs in_{};
    Debounced start_, stop_, reset_, safety_, home_front_, home_rear_, part_clear_, waste_clear_;
    Outputs out_{};
    AxisState axis_[ctrl::axis::count];
    bool homed_ = false;
    bool priming_ = false;          // current cycle pass is the prime
    bool home_sub_done_[2] = {false, false};
    Job job_{};
    bool job_loaded_ = false;
    Counters counters_{};
    Settings settings_{};
    std::uint64_t now_us_ = 0;
    std::uint64_t phase_started_us_ = 0;
    std::uint64_t homing_started_us_ = 0;
    std::uint64_t beep_until_us_ = 0;
    std::uint64_t drives_enabled_at_us_ = 0;
    std::int32_t feed_target_um_ = 0;
    std::int64_t feed_encoder_start_[2] = {0, 0};
    bool correction_done_ = false;
    Axis jog_axis_ = ctrl::axis::blade_front;
    bool jog_forward_ = false;
    std::int32_t jog_distance_um_ = 0;
    std::int32_t jog_speed_um_s_ = 0;
    State stop_return_state_ = State::Ready;

    // Explicit wire-body model (F07). Positions are commanded logical X
    // micrometers relative to the cut plane; the model rejects any planned
    // motion that would overlap the stock and the severed piece, lose a nip
    // grip, score the wrong end, or restore the stock before clearance.
    std::int32_t stock_tip_um_ = 0;
    std::int32_t piece_tail_um_ = 0;
    std::int32_t piece_len_um_ = 0;
    bool piece_present_ = false;
    std::int32_t stock_restore_um_ = 0;
    // Discharge evidence (M27): edge history is captured continuously from the
    // start of the ejection observation window - never restarted after the only
    // useful transition has passed. Validation requires a debounced
    // present->clear pair whose present edge lies inside the window.
    bool part_present_seen_ = false;
    bool part_clear_confirmed_ = false;
    std::uint64_t clear_started_us_ = 0;      // observation-window start (ejection)
    std::uint64_t part_present_at_us_ = 0;    // last CONFIRMED present edge
    std::uint64_t part_clear_at_us_ = 0;      // last CONFIRMED clear edge (must follow present)
    bool part_level_confirmed_ = true;        // last confirmed (debounce-stable) part_clear level
    // Waste-path evidence: every released stub/slug must transit the chute.
    bool waste_present_seen_ = false;
    bool waste_clear_confirmed_ = false;
    std::uint64_t waste_started_us_ = 0;
    std::uint64_t waste_present_at_us_ = 0;
    std::uint64_t waste_clear_at_us_ = 0;
    bool waste_level_confirmed_ = true;       // last confirmed waste_clear level
    enum class WasteNext : std::uint8_t { PrimeDone, AfterLead, AfterTail };
    WasteNext waste_next_ = WasteNext::PrimeDone;
    bool piece_done_prime_ = false;
    TrackedFeed tracked_[2];
    TrackedFeed tracked_piece_;
    // Per-piece D cross-check window (review F04).
    std::int64_t piece_d_start_ = 0;
    bool piece_d_watch_ = false;
    bool fixture_mode_ = false; // RAM-only service fixture mode

    // Commissioning polarity validation (F33).
    bool commissioning_ = false;
    Axis commission_axis_ = ctrl::axis::feed_left;
    bool commission_forward_ = true;
    std::int64_t commission_encoder_start_ = 0;
    bool commission_sensor_start_ = false;
    CommandResult commission_result_ = CommandResult::Unsupported;
};

} // namespace starz::fw
