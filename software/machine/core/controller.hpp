#pragma once
// Machine controller: operator buttons, safety supervision, blade homing,
// cut/strip cycle sequencing, encoder-verified feed length and host commands.
// Runs at ctrl::mcu::control_period_us on the target; the host tests drive
// tick() with a simulated Board.
#include <cstdint>
#include "types.hpp"
#include "board.hpp"
#include "profile.hpp"

namespace starz::fw {

enum class CommandResult : std::uint8_t {
    Accepted = 0, InvalidArgument = 1, WrongState = 2, Busy = 3, Faulted = 4,
    NotHomed = 5, NoJob = 6, Unsupported = 7, MalformedFrame = 8, StorageError = 9
};

enum class Phase : std::uint8_t {
    Idle = 0,
    // homing (both blade axes)
    HomeLeave, HomeSeek, HomeBackoff, HomeApproach,
    // cycle; the first pass after START is the prime (feed stock clearance,
    // sever, strip the lead) and does not count a piece.
    EnableSettle, Feed, FeedVerify, FeedCorrect, Cut, OpenAfterCut,
    StripPosition, StripClose, StripPull, Open, Eject, PieceDone,
    Jog,
    Stop
};

struct Counters {
    std::uint32_t pieces_done = 0;   // in current job
    std::uint32_t pieces_scrap = 0;  // partial pieces discarded after a stop/fault
    std::int32_t last_residual_um = 0; // target minus measured on the last verified feed
    std::int32_t worst_residual_um = 0;
};

struct Telemetry {
    State state;
    Phase phase;
    std::uint16_t faults;
    Inputs inputs;
    Outputs outputs;
    bool job_loaded;
    Job job;
    Counters counters;
    std::int32_t blade_um[2];      // travel from open, commanded
    std::int64_t feed_um[2];       // cumulative commanded transport, left/right
    std::int64_t encoder_counts[2];
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
    CommandResult set_calibration(std::int32_t left_counts_per_metre, std::int32_t right_counts_per_metre);

    Telemetry telemetry() const;
    State state() const noexcept { return state_; }
    Phase phase() const noexcept { return phase_; }
    std::uint16_t faults() const noexcept { return faults_; }
    const Counters& counters() const noexcept { return counters_; }

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

    // supervision
    void sample_inputs();
    void supervise_safety();
    void enter_fault(std::uint16_t bits);
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
    std::int32_t strip_travel_um() const;
    std::int32_t measured_um(std::uint8_t side, std::int64_t counts_delta) const;
    std::int32_t blade_travel_um(Axis a) const;

    Board& board_;
    State state_ = State::PowerOn;
    Phase phase_ = Phase::Idle;
    std::uint16_t faults_ = 0;
    Inputs raw_{};
    Inputs in_{};
    Debounced start_, stop_, reset_, safety_, home_front_, home_rear_;
    Outputs out_{};
    AxisState axis_[ctrl::axis::count];
    bool homed_ = false;
    bool priming_ = false;          // current cycle pass is the prime
    bool cycle_started_ = false;    // PieceDone has launched at least one pass
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
};

} // namespace starz::fw
