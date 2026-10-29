// Machine software host tests: step timing exactness, job validation, homing,
// production cycle geometry, encoder verification, stop/pause, fault latching,
// protocol round-trip and the external-audit regression scenarios (F05, F07,
// F08, F09, F11, F12, F33). Runs with the simulated Board; no hardware.
#include "controller.hpp"
#include "crc16.hpp"
#include "protocol.hpp"
#include "settings_journal.hpp"
#include "sim_board.hpp"
#include "material_plant.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace starz;
using namespace starz::fw;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)
#define REQUIRE(cond) do { if (!(cond)) { std::printf("FATAL %s:%d: %s\n", __FILE__, __LINE__, #cond); return 1; } } while (0)

namespace {

std::int32_t step_error_budget(double moves) {
    return static_cast<std::int32_t>(moves * 1000.0 / ctrl::axis::feed_steps_per_mm);
}

template <class B>
struct RigT {
    B b;
    Controller c{b};
    RigT() { c.init(); }
    // Run the 1 kHz loop for ms milliseconds.
    void run_ms(std::uint32_t ms) {
        for (std::uint32_t i = 0; i < ms; ++i) { c.tick(); b.advance(ctrl::mcu::control_period_us); }
    }
    bool run_until(State s, std::uint32_t max_ms) {
        for (std::uint32_t i = 0; i < max_ms; ++i) { if (c.state() == s) return true; run_ms(1); }
        return c.state() == s;
    }
    bool run_until_phase(Phase p, std::uint32_t max_ms) {
        for (std::uint32_t i = 0; i < max_ms; ++i) {
            if (c.state() == State::Fault) return false;
            if (c.phase() == p) return true;
            run_ms(1);
        }
        return c.phase() == p;
    }
    void press(bool Inputs::*btn, std::uint32_t hold_ms) { b.field.*btn = true; run_ms(hold_ms); b.field.*btn = false; run_ms(ctrl::timing::input_debounce_ms + 5); }
    void boot_and_home() {
        run_ms(ctrl::timing::input_debounce_ms + 5);
        press(&Inputs::start, ctrl::timing::start_hold_ms + ctrl::timing::input_debounce_ms + 10);
        run_until(State::Ready, ctrl::timing::homing_timeout_ms + 1000);
    }
    bool load_and_approve(const Job& j) {
        return c.load_job(j) == CommandResult::Accepted && c.approve() == CommandResult::Accepted;
    }
};

// Default rig: the independent material plant (M27) with physically honest
// sensor kinematics - production behaviour is tested here.
using Rig = RigT<sim::MaterialPlant>;
// Legacy rig: the original motor-coupled fixture. Its apply_step() advances
// each encoder from commanded steps (the stock-side counterexample defect), so
// it is retained ONLY for negative-control and board-level tests that do not
// claim machine function.
using LegacyRig = RigT<sim::SimBoard>;

Job good_job() {
    Job j{};
    j.length_um = 300000; j.strip_lead_um = 8000; j.strip_tail_um = 6000;
    j.strip_depth_um = 600; j.wire_diameter_um = 2500; j.quantity = 3;
    return j;
}

Job no_strip_job(std::int32_t length_um = 300000, std::uint32_t quantity = 1) {
    Job j = good_job();
    j.length_um = length_um; j.strip_lead_um = 0; j.strip_tail_um = 0; j.strip_depth_um = 0; j.quantity = quantity;
    return j;
}

std::string read_text(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string between(const std::string& text, const std::string& begin, const std::string& end) {
    const std::size_t a = text.find(begin);
    if (a == std::string::npos) return {};
    const std::size_t b = text.find(end, a);
    if (b == std::string::npos) return {};
    return text.substr(a, b - a);
}

// ---------------------------------------------------------------- profile
int test_profile() {
    // Every wire period must satisfy the driver minimum (>= 1 us high/low), the
    // total duration must match the trapezoid within one tick per pulse, and
    // the step count must be exact.
    for (double v : {500.0, 7958.0, 1250.0}) {
        for (std::uint32_t n : {1u, 2u, 7u, 100u, 4775u}) {
            StepStreamer s;
            MoveSpec m; m.steps = n; m.steps_per_s = v; m.steps_per_s2 = 1500 * 15.915; m.forward = true;
            REQUIRE(s.begin(m, ctrl::axis::pio_clock_hz));
            std::uint64_t pulses = 0; StepRun r{};
            std::uint32_t last_y = 0; bool first = true; unsigned direction_changes = 0; int trend = 0;
            while (s.pop(r)) {
                CHECK(r.count >= 1 && r.count <= max_run_pulses);
                CHECK(r.half_period_ticks >= min_half_period_ticks);
                pulses += r.count;
                if (!first) {
                    // Error diffusion dithers between adjacent tick values at cruise; only
                    // changes of more than one tick count as a speed trend.
                    const std::int64_t dy = static_cast<std::int64_t>(r.half_period_ticks) - static_cast<std::int64_t>(last_y);
                    const int d = dy < -1 ? 1 : (dy > 1 ? -1 : 0);
                    if (d != 0 && trend != 0 && d != trend) ++direction_changes;
                    if (d != 0) trend = d;
                }
                first = false; last_y = r.half_period_ticks;
            }
            CHECK(pulses == n);
            // Compare emitted ticks with the ideal trapezoid duration.
            const double a = m.steps_per_s2;
            const double n_acc = std::min(static_cast<double>(n) / 2.0, v * v / (2 * a));
            const double vp = std::sqrt(2 * a * n_acc);
            const double t_ideal = 2 * vp / a + (n - 2 * n_acc) / vp;
            const double t_emitted = static_cast<double>(s.issued_ticks()) / ctrl::axis::pio_clock_hz;
            // Integer ramp lengths shift the duration by at most a few pulse periods.
            CHECK(n < 2 || std::fabs(t_emitted - t_ideal) <= 3.0 / vp + 1e-6 * n);
            // One accel and one decel: at most one trend reversal.
            CHECK(direction_changes <= 1);
        }
    }
    // Controlled stop: pulses after begin_stop must decelerate monotonically and
    // finish within v^2/(2a) steps.
    {
        StepStreamer s; MoveSpec m; m.steps = 100000; m.steps_per_s = 7958; m.steps_per_s2 = 1500 * 15.915;
        REQUIRE(s.begin(m, ctrl::axis::pio_clock_hz));
        StepRun r{}; std::uint32_t before = 0;
        while (before < 20000 && s.pop(r)) before += r.count;
        s.begin_stop();
        std::uint32_t after = 0, last_y = 0;
        while (s.pop(r)) { CHECK(r.half_period_ticks + 1 >= last_y); last_y = r.half_period_ticks; after += r.count; }
        const double v0 = 7958.0, a = 1500 * 15.915;
        CHECK(after <= static_cast<std::uint32_t>(v0 * v0 / (2 * a)) + 2);
        CHECK(!s.active());
    }
    return 0;
}

// --------------------------------------------------------------- validate
int test_validate() {
    CHECK(validate(good_job()) == JobError::None);
    Job j = good_job(); j.length_um = 199999; CHECK(validate(j) == JobError::Length);
    j = good_job(); j.length_um = 3000001; CHECK(validate(j) == JobError::Length);
    j = good_job(); j.strip_lead_um = 2999; CHECK(validate(j) == JobError::StripLead);
    j = good_job(); j.strip_tail_um = 25001; CHECK(validate(j) == JobError::StripTail);
    j = good_job(); j.strip_depth_um = 50; CHECK(validate(j) == JobError::Depth);
    j = good_job(); j.strip_depth_um = 1250; CHECK(validate(j) == JobError::Depth); // would sever 2.5 mm wire
    j = good_job(); j.wire_diameter_um = 1100; CHECK(validate(j) == JobError::WireDiameter);
    j = good_job(); j.wire_diameter_um = 4600; CHECK(validate(j) == JobError::WireDiameter);
    j = good_job(); j.quantity = 0; CHECK(validate(j) == JobError::Quantity);
    j = good_job(); j.length_um = 200000; j.strip_lead_um = 25000; j.strip_tail_um = 25000; CHECK(validate(j) == JobError::None);
    j.length_um = 200000; j.strip_lead_um = 25000; j.strip_tail_um = 25000; j.quantity = 1;
    j.length_um = 59999; CHECK(validate(j) == JobError::Length);
    j = good_job(); j.strip_lead_um = 0; j.strip_tail_um = 0; j.strip_depth_um = 0; CHECK(validate(j) == JobError::None);
    return 0;
}

// ------------------------------------------------------------------ homing
int test_homing() {
    Rig r;
    r.run_ms(ctrl::timing::input_debounce_ms + 5);
    CHECK(r.c.state() == State::Unhomed);
    CHECK(r.b.outputs.drives_disabled);
    r.press(&Inputs::start, 150);
    CHECK(r.c.state() == State::Homing);
    REQUIRE(r.run_until(State::Ready, 25000));
    // Blades stopped on the sensor edge within one step of the 200 um window.
    for (int i = 0; i < 2; ++i) {
        CHECK(r.b.blade_true_um[i] <= r.b.sensor_window_um);
        CHECK(r.b.blade_true_um[i] > r.b.sensor_window_um - 1000.0 / ctrl::axis::blade_steps_per_mm - 1e-6);
    }
    // Commanded position is zero at home.
    const Telemetry t = r.c.telemetry();
    CHECK(t.blade_um[0] == 0 && t.blade_um[1] == 0);

    // Homing starting on the sensor: leave, then re-seek.
    Rig r2; r2.b.blade_true_um[0] = 50; r2.b.blade_true_um[1] = 100;
    r2.boot_and_home();
    CHECK(r2.c.state() == State::Ready);

    // No sensor within search distance -> homing_failed.
    Rig r3; r3.b.blade_true_um[0] = 40000; r3.b.blade_true_um[1] = 5000;
    r3.run_ms(25); r3.press(&Inputs::start, 150);
    REQUIRE(r3.run_until(State::Fault, 25000));
    CHECK(r3.c.faults() & fault::homing_failed);
    CHECK(r3.b.outputs.drives_disabled);

    // Each individual motion phase is shorter than 20 s, but their combined
    // duration exceeds the whole-homing deadline.
    Rig r4; r4.b.blade_true_um[0] = 29000; r4.b.blade_true_um[1] = 29000;
    r4.run_ms(25); r4.press(&Inputs::start, 150);
    REQUIRE(r4.run_until(State::Fault, ctrl::timing::homing_timeout_ms + 1000));
    CHECK(r4.c.faults() & fault::homing_failed);
    return 0;
}

// ------------------------------------------------------------------- cycle
int test_cycle() {
    Rig r;
    r.boot_and_home();
    REQUIRE(r.c.state() == State::Ready);
    const Job j = good_job();
    REQUIRE(r.load_and_approve(j));
    CHECK(r.c.start() == CommandResult::Accepted);
    CHECK(r.c.state() == State::Running);
    REQUIRE(r.run_until(State::Ready, 200000));
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == j.quantity);
    CHECK(t.counters.pieces_accepted == j.quantity);
    CHECK(t.counters.pieces_scrap == 0);
    CHECK(t.job_loaded); // completed recipe remains available for panel-only repeat batches
    CHECK(t.approved);   // approval survives the completed batch
    // Blades open at the end, drives still enabled (Ready).
    CHECK(t.blade_um[0] == 0 && t.blade_um[1] == 0);
    // Right feed net advance: prime stock clearance (the prime stub is swept,
    // not fed), plus, per piece, the L+3 feed, the s2 backfeed, the s2+3 pull
    // and the ejection.
    const double feed_len = j.length_um + spec::strip_clear_um;
    const double expected = spec::stock_clear_um +
        j.quantity * (feed_len + 3000.0 + ctrl::job::ejection_um);
    CHECK(std::fabs((r.b.feed_true_um[1] - 0.0) - expected) <= step_error_budget(2.0 + 4.0 * j.quantity));
    // No pulse violated the driver minimum.
    CHECK(r.b.min_half_period_seen >= min_half_period_ticks);
    // Encoder residual within three counts of zero with an ideal roller: two
    // independent count quantisations plus one accumulated fraction.
    CHECK(std::abs(t.counters.worst_residual_um) <= static_cast<std::int32_t>(3 * ctrl::enc::nominal_um_per_count));
    CHECK(r.b.stores >= 1); // pieces_total persisted at job end
    return 0;
}

int test_cycle_no_strip_and_correction() {
    Rig r; r.b.slip_ppm[0] = -3000; r.b.slip_ppm[1] = -3000; // roller reads 0.3 % short of command
    r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job(300000, 2))); // 0.9 mm short, > budget, < max_slip
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Ready, 120000));
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == 2);
    CHECK(t.counters.pieces_accepted == 2);
    // One corrective move brings the encoder-measured length within the deadband.
    CHECK(std::abs(t.counters.last_residual_um) <= static_cast<std::int32_t>(2 * ctrl::enc::nominal_um_per_count));
    return 0;
}

int test_slip_fault_and_no_wire() {
    Rig r; r.b.slip_ppm[0] = -20000; // 2 % short -> 6 mm on 300 mm > 1.5 mm
    r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job()));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Fault, 60000));
    CHECK(r.c.faults() & fault::length_slip);
    CHECK(r.c.telemetry().job_loaded); // job retained across the fault
    CHECK(r.b.outputs.drives_disabled);

    Rig r2; r2.b.wire_present = false;
    r2.boot_and_home();
    REQUIRE(r2.load_and_approve(no_strip_job()));
    r2.press(&Inputs::start, 150);
    REQUIRE(r2.run_until(State::Fault, 60000));
    CHECK(r2.c.faults() & fault::no_wire);
    // RESET held clears the fault into Unhomed; short press does not.
    r2.press(&Inputs::reset, 100);
    CHECK(r2.c.state() == State::Fault);
    r2.press(&Inputs::reset, 600);
    CHECK(r2.c.state() == State::Unhomed);
    CHECK(r2.c.faults() == 0);

    // Both measuring wheels must agree; an opposite scale error is not
    // masked by averaging their residuals.
    Rig r3; r3.b.slip_ppm[0] = -3000; r3.b.slip_ppm[1] = 3000;
    r3.boot_and_home();
    REQUIRE(r3.load_and_approve(no_strip_job()));
    r3.press(&Inputs::start, 150);
    REQUIRE(r3.run_until(State::Fault, 60000));
    CHECK(r3.c.faults() & fault::length_slip);
    return 0;
}

int test_stop_pause_resume() {
    Rig r; r.boot_and_home();
    Job j = no_strip_job(2000000, 5);
    REQUIRE(r.load_and_approve(j));
    r.press(&Inputs::start, 150);
    r.run_ms(2000); // mid-feed of the first production piece or prime
    CHECK(r.c.state() == State::Running);
    r.b.field.stop_ok = false; // NC STOP pressed
    r.run_ms(50);
    CHECK(r.c.state() == State::Stopping || r.c.state() == State::Paused);
    REQUIRE(r.run_until(State::Paused, 20000));
    CHECK(!r.b.outputs.drives_disabled); // controlled stop keeps holding torque
    // START ignored while STOP still pressed.
    r.press(&Inputs::start, 150);
    CHECK(r.c.state() == State::Paused);
    r.b.field.stop_ok = true; r.run_ms(50);
    r.press(&Inputs::start, 150);
    CHECK(r.c.state() == State::Running);
    REQUIRE(r.run_until(State::Ready, 400000));
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == 5);
    CHECK(t.counters.pieces_accepted == 5);
    CHECK(t.counters.pieces_scrap == 1);
    return 0;
}

int test_safety_and_alarm() {
    Rig r; r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job()));
    r.press(&Inputs::start, 150);
    r.run_ms(500);
    r.b.field.safety_ok = false;
    r.c.tick();
    CHECK(r.b.outputs.drives_disabled);           // same tick as the raw loss
    for (std::uint8_t a = 0; a < ctrl::axis::count; ++a) CHECK(r.b.steps_idle(static_cast<Axis>(a)));
    r.run_ms(50);
    CHECK(r.c.state() == State::Fault);
    CHECK(r.c.faults() & fault::safety_open);
    // Cannot reset while the loop is open.
    r.press(&Inputs::reset, 600);
    CHECK(r.c.state() == State::Fault);
    r.b.field.safety_ok = true; r.run_ms(50);
    r.press(&Inputs::reset, 600);
    CHECK(r.c.state() == State::Unhomed);         // homing lost after a fault

    Rig r2; r2.boot_and_home();
    r2.b.field.alm[2] = true; r2.run_ms(5);
    CHECK(r2.c.state() == State::Fault);
    CHECK(r2.c.faults() & fault::alm_feed_left);
    return 0;
}

int test_board_fault_latches() {
    // A watchdog reset starts in a latched fault, never in an inferred-ready
    // state. Healthy, explicit RESET returns only to Unhomed.
    sim::SimBoard boot_board;
    boot_board.boot_fault_bits = fault::watchdog_reset;
    Controller boot_controller{boot_board};
    boot_controller.init();
    CHECK(boot_controller.state() == State::Fault);
    CHECK(boot_controller.faults() == fault::watchdog_reset);
    for (std::uint32_t i = 0; i < 25; ++i) { boot_controller.tick(); boot_board.advance(1000); }
    CHECK(boot_controller.reset() == CommandResult::Accepted);
    CHECK(boot_controller.state() == State::Unhomed);

    Rig r; r.boot_and_home();
    r.b.encoder_ok[0] = false;
    r.b.runtime_fault_bits = fault::encoder_invalid;
    r.run_ms(1);
    CHECK(r.c.state() == State::Fault);
    CHECK(r.c.faults() & fault::encoder_invalid);
    r.b.clear_latches_allowed = false;
    CHECK(r.c.reset() == CommandResult::WrongState);
    r.b.clear_latches_allowed = true;
    CHECK(r.c.reset() == CommandResult::Accepted);
    CHECK(r.c.state() == State::Unhomed);

    // PIO RX FIFO overflow is independently latched and must remain visible
    // as that exact fault class rather than degrading into encoder-invalid.
    Rig overflow; overflow.boot_and_home();
    overflow.b.runtime_fault_bits = fault::encoder_overflow;
    overflow.run_ms(1);
    CHECK(overflow.c.state() == State::Fault);
    CHECK(overflow.c.faults() == fault::encoder_overflow);
    return 0;
}

int test_jog_and_commands() {
    Rig r; r.run_ms(25);
    CHECK(r.c.jog(ctrl::axis::blade_front, true, 1000, 5000) == CommandResult::NotHomed);
    CHECK(r.c.jog(ctrl::axis::feed_left, true, 50000, 100000) == CommandResult::Accepted);
    CHECK(r.c.state() == State::Jogging);
    const std::uint64_t feed_pulses_before_enable = r.b.pulses[ctrl::axis::feed_left];
    r.run_ms(ctrl::timing::drive_enable_settle_ms - 1);
    CHECK(r.c.phase() == Phase::EnableSettle);
    CHECK(r.b.pulses[ctrl::axis::feed_left] == feed_pulses_before_enable);
    r.run_ms(100);
    CHECK(r.b.pulses[ctrl::axis::feed_left] > feed_pulses_before_enable);
    REQUIRE(r.run_until(State::Unhomed, 10000));
    CHECK(std::fabs(r.b.feed_true_um[0] - 50000) < 100);
    r.boot_and_home();
    CHECK(r.c.jog(ctrl::axis::blade_front, true, 4300, 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, true, 2000, 5000) == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Ready, 10000));
    CHECK(r.c.telemetry().blade_um[0] == 2000);
    CHECK(r.c.start() == CommandResult::NoJob);
    CHECK(r.c.set_calibration(42000, 41600, 41000) == CommandResult::Accepted);
    CHECK(r.c.set_calibration(38000, 41600, 41000) == CommandResult::InvalidArgument);
    const Job saved = good_job();
    CHECK(r.c.load_job(saved) == CommandResult::Accepted);
    // Calibration and the valid screenless operating recipe survive a re-init.
    Controller c2(r.b); c2.init();
    CHECK(c2.telemetry().settings.counts_per_metre[0] == 42000);
    CHECK(c2.telemetry().job_loaded);
    CHECK(c2.telemetry().job.length_um == saved.length_um && c2.telemetry().job.quantity == saved.quantity);
    CHECK(c2.clear_job() == CommandResult::Accepted);
    Controller c3(r.b); c3.init();
    CHECK(!c3.telemetry().job_loaded);
    return 0;
}

int test_settings_storage_failure() {
    Rig r; r.run_ms(25);
    const Job recipe = good_job();
    r.b.store_settings_allowed = false;
    CHECK(r.c.load_job(recipe) == CommandResult::StorageError);
    CHECK(!r.c.telemetry().job_loaded);
    const Settings before = r.c.telemetry().settings;
    CHECK(r.c.set_calibration(42000, 41600, 41000) == CommandResult::StorageError);
    const Settings after = r.c.telemetry().settings;
    CHECK(after.counts_per_metre[0] == before.counts_per_metre[0]);
    CHECK(after.counts_per_metre[1] == before.counts_per_metre[1]);

    r.b.store_settings_allowed = true;
    CHECK(r.c.load_job(recipe) == CommandResult::Accepted);
    CHECK(r.c.telemetry().job_loaded);
    r.b.store_settings_allowed = false;
    CHECK(r.c.clear_job() == CommandResult::StorageError);
    CHECK(r.c.telemetry().job_loaded);
    CHECK(r.c.telemetry().job.length_um == recipe.length_um);
    return 0;
}

int test_settings_journal() {
    auto make_settings = [](std::int32_t left, std::int32_t right) {
        Settings settings{};
        settings.counts_per_metre[0] = left;
        settings.counts_per_metre[1] = right;
        settings.crc = crc16(reinterpret_cast<const std::uint8_t*>(&settings), offsetof(Settings, crc));
        return settings;
    };
    auto make_record = [&](std::uint32_t generation, std::int32_t left, std::int32_t right) {
        journal::Record record{};
        record.magic = journal::record_magic;
        record.generation = generation;
        record.settings = make_settings(left, right);
        record.record_crc = journal::record_crc(record);
        return record;
    };

    journal::Record records[2]{};
    std::uint32_t latches[2] = {0xffffffffu, 0xffffffffu};
    // An erased / incomplete candidate without its distinct commit write can
    // never replace the old settings record.
    records[0] = make_record(41, 41000, 40900);
    CHECK(!journal::valid(records[0], latches[0]));
    CHECK(journal::select(records, latches, 2) == -1);

    latches[0] = journal::committed;
    CHECK(journal::valid(records[0], latches[0]));
    CHECK(journal::select(records, latches, 2) == 0);

    records[1] = make_record(42, 42000, 41900);
    // The record page is present but the commit page is still erased.
    CHECK(!journal::valid(records[1], latches[1]));
    CHECK(journal::select(records, latches, 2) == 0);
    // A torn/corrupt record is invalid even if its commit latch was written.
    latches[1] = journal::committed;
    records[1].settings.counts_per_metre[0] = 1;
    CHECK(!journal::valid(records[1], latches[1]));
    CHECK(journal::select(records, latches, 2) == 0);

    records[1] = make_record(42, 42000, 41900);
    CHECK(journal::valid(records[1], latches[1]));
    CHECK(journal::select(records, latches, 2) == 1);

    // A valid generation after uint32 wrap is correctly newer.
    records[0] = make_record(0xfffffff0u, 43000, 42900);
    records[1] = make_record(10u, 44000, 43900);
    CHECK(journal::select(records, latches, 2) == 1);
    return 0;
}

int test_protocol() {
    Rig r; r.boot_and_home();
    proto::Frame req{}; req.msg = proto::Msg::LoadJob; req.seq = 7; req.len = proto::load_job_len;
    proto::encode_job(good_job(), req.payload);
    std::uint8_t buf[proto::max_frame];
    const std::size_t n = proto::encode(req, buf, sizeof buf);
    REQUIRE(n == 7 + proto::load_job_len);
    proto::Decoder d; bool got = false;
    for (std::size_t i = 0; i < n; ++i) got = d.feed(buf[i]) || got;
    REQUIRE(got);
    proto::Frame reply{};
    proto::dispatch(r.c, d.frame(), reply);
    CHECK(reply.msg == proto::Msg::Ack && reply.seq == 7);
    CHECK(r.c.telemetry().job_loaded);
    // Corrupted byte -> dropped, not delivered.
    buf[10] ^= 0x40; got = false;
    for (std::size_t i = 0; i < n; ++i) got = d.feed(buf[i]) || got;
    CHECK(!got && d.dropped() == 1);
    // A stale partial frame is explicitly discarded before the next packet.
    d.feed(proto::sync0); d.feed(proto::sync1); d.feed(static_cast<std::uint8_t>(proto::Msg::Start));
    d.reset();
    buf[10] ^= 0x40; // restore the valid LOAD_JOB frame
    got = false;
    for (std::size_t i = 0; i < n; ++i) got = d.feed(buf[i]) || got;
    CHECK(got);
    // The transport timeout discards a partial frame before accepting the
    // next request, matching the bounded Pico USB service behavior.
    proto::TimedDecoder timed{100000};
    timed.feed(proto::sync0, 0); timed.feed(proto::sync1, 0);
    timed.feed(static_cast<std::uint8_t>(proto::Msg::Start), 0);
    timed.expire(100001);
    proto::Frame timed_request{}; timed_request.msg = proto::Msg::GetStatus; timed_request.seq = 11;
    std::uint8_t timed_wire[proto::max_frame]{};
    const std::size_t timed_count = proto::encode(timed_request, timed_wire, sizeof timed_wire);
    bool timed_got = false;
    for (std::size_t i = 0; i < timed_count; ++i) timed_got = timed.feed(timed_wire[i], 100002) || timed_got;
    CHECK(timed_got && timed.frame().msg == proto::Msg::GetStatus && timed.frame().seq == 11);
    // Status round trip.
    proto::Frame sreq{}; sreq.msg = proto::Msg::GetStatus;
    proto::dispatch(r.c, sreq, reply);
    Telemetry t{};
    REQUIRE(proto::decode_status(reply.payload, reply.len, t));
    CHECK(t.state == State::Ready && t.job_loaded);
    CHECK(!t.approved); // recipe loaded but not yet approved
    CHECK(t.settings.counts_per_metre[0] == ctrl::enc::nominal_counts_per_metre);
    CHECK(t.job.length_um == good_job().length_um && t.job.strip_lead_um == good_job().strip_lead_um &&
          t.job.strip_tail_um == good_job().strip_tail_um && t.job.strip_depth_um == good_job().strip_depth_um &&
          t.job.wire_diameter_um == good_job().wire_diameter_um && t.job.quantity == good_job().quantity);
    CHECK(!proto::decode_status(reply.payload, static_cast<std::uint8_t>(proto::status_len - 1), t));
    std::uint8_t malformed_status[proto::status_len]{};
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[0] = static_cast<std::uint8_t>(State::Jogging) + 1;
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[1] = static_cast<std::uint8_t>(Phase::WasteClear) + 1;
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[6] |= 0x40; // reserved input bit (6)
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[8] |= 0x40; // reserved output bit (6; bit 5 is waste_sweep)
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[9] = 2; // job_loaded must be canonical
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    proto::put_u32(malformed_status + 102, 0); // loaded recipe cannot have zero quantity
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    Telemetry wide{};
    wide.feed_um[0] = 3000000000ll; wide.feed_um[1] = -3000000000ll;
    wide.encoder_counts[0] = 3000000000ll; wide.encoder_counts[1] = -3000000000ll;
    wide.faults = fault::part_not_cleared | fault::length_budget;
    wide.counters.pieces_accepted = 1234;
    wide.inputs.part_clear = true;
    wide.approved = true;
    std::uint8_t status[proto::status_len]{};
    proto::encode_status(wide, status);
    Telemetry decoded{};
    REQUIRE(proto::decode_status(status, proto::status_len, decoded));
    CHECK(decoded.feed_um[0] == wide.feed_um[0] && decoded.feed_um[1] == wide.feed_um[1]);
    CHECK(decoded.encoder_counts[0] == wide.encoder_counts[0] && decoded.encoder_counts[1] == wide.encoder_counts[1]);
    CHECK(decoded.faults == wide.faults);
    CHECK(decoded.counters.pieces_accepted == wide.counters.pieces_accepted);
    CHECK(decoded.inputs.part_clear && decoded.approved);
    proto::Frame malformed{}; malformed.msg = proto::Msg::Start; malformed.seq = 8; malformed.len = 1; malformed.payload[0] = 0;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::MalformedFrame));
    malformed = proto::Frame{}; malformed.msg = proto::Msg::Jog; malformed.seq = 9; malformed.len = proto::jog_len;
    malformed.payload[0] = 0; malformed.payload[1] = 2;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::InvalidArgument));
    // Approve/commission/part-clear requests are length-checked and routed.
    malformed = proto::Frame{}; malformed.msg = proto::Msg::Approve; malformed.seq = 12;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Ack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::Accepted));
    CHECK(r.c.telemetry().approved);
    malformed = proto::Frame{}; malformed.msg = proto::Msg::Approve; malformed.seq = 13; malformed.len = 1;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::MalformedFrame));
    malformed = proto::Frame{}; malformed.msg = proto::Msg::CommissionAxis; malformed.seq = 14; malformed.len = proto::commission_len;
    malformed.payload[0] = 4; malformed.payload[1] = 1; // invalid axis
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::InvalidArgument));
    malformed = proto::Frame{}; malformed.msg = proto::Msg::SetPartClear; malformed.seq = 15; malformed.len = proto::set_part_clear_len;
    malformed.payload[0] = 1;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Ack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::Accepted));
    CHECK(r.c.telemetry().settings.part_clear_check == 1);
    CHECK(!r.c.telemetry().approved); // a production-setting change invalidates approval
    // Revision-7 field round-trip: piece station, fixture state and the waste
    // channel must survive encode/decode exactly (blind spot regression).
    Telemetry wide2{};
    wide2.piece_encoder_counts = -1234567890123ll;
    wide2.counters.pieces_fixture = 77;
    wide2.settings.counts_per_metre[2] = 41234;
    wide2.fixture_mode = true;
    wide2.inputs.waste_clear = true;
    wide2.outputs.waste_sweep = true;
    std::uint8_t status2[proto::status_len]{};
    proto::encode_status(wide2, status2);
    Telemetry decoded2{};
    REQUIRE(proto::decode_status(status2, proto::status_len, decoded2));
    CHECK(decoded2.piece_encoder_counts == wide2.piece_encoder_counts);
    CHECK(decoded2.counters.pieces_fixture == 77);
    CHECK(decoded2.settings.counts_per_metre[2] == 41234);
    CHECK(decoded2.fixture_mode && decoded2.inputs.waste_clear && decoded2.outputs.waste_sweep);
    // SetFixtureMode dispatch: fixture mode permits disabling handling
    // verification but never persists approval; leaving it restores the
    // production default.
    malformed = proto::Frame{}; malformed.msg = proto::Msg::SetFixtureMode; malformed.seq = 16;
    malformed.len = proto::set_fixture_len; malformed.payload[0] = 1;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Ack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::Accepted));
    CHECK(r.c.telemetry().fixture_mode);
    malformed = proto::Frame{}; malformed.msg = proto::Msg::SetPartClear; malformed.seq = 17;
    malformed.len = proto::set_part_clear_len; malformed.payload[0] = 0;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Ack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::Accepted));
    CHECK(r.c.telemetry().settings.part_clear_check == 0);
    malformed = proto::Frame{}; malformed.msg = proto::Msg::SetFixtureMode; malformed.seq = 18;
    malformed.len = proto::set_fixture_len; malformed.payload[0] = 0;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Ack);
    CHECK(!r.c.telemetry().fixture_mode);
    CHECK(r.c.telemetry().settings.part_clear_check == 1); // production default restored
    malformed = proto::Frame{}; malformed.msg = proto::Msg::SetFixtureMode; malformed.seq = 19;
    malformed.len = proto::set_fixture_len; malformed.payload[0] = 2;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::InvalidArgument));
    // Wire-level CRC check vector.
    CHECK(crc16(reinterpret_cast<const std::uint8_t*>("123456789"), 9) == 0x29B1);
    return 0;
}

// ------------------------------------------------------------------- F05
// The RX-joined quadrature state machine must never touch the TX path: the
// initial AB state is loaded with a forced `set x` instead of a blocking put.
int test_f05_quadrature_init() {
    const std::string root = STARZ_MACHINE_SOURCE_DIR;
    const std::string pio = read_text(root + "/rp2350/quadrature.pio");
    const std::string board = read_text(root + "/rp2350/board_rp2350.cpp");
    const std::string factory = read_text(root + "/rp2350/factory_test.cpp");
    REQUIRE(!pio.empty() && !board.empty() && !factory.empty());

    CHECK(pio.find("pio_sm_put_blocking(pio") == std::string::npos);
    CHECK(pio.find("pull") == std::string::npos);      // no TX word is consumed at all
    CHECK(pio.find("out ") == std::string::npos);      // no OSR/out path either
    CHECK(pio.find("pio_encode_set(pio_x") != std::string::npos);
    CHECK(board.find("pio_sm_put_blocking(pio") == std::string::npos);
    CHECK(factory.find("pio_sm_put_blocking(pio") == std::string::npos);

    const std::string restart_board = between(board, "void BoardRp2350::restart_encoder_sm", "bool BoardRp2350::load_settings");
    REQUIRE(!restart_board.empty());
    CHECK(restart_board.find("pio_sm_put_blocking(pio") == std::string::npos);
    CHECK(restart_board.find("pio_encode_set(pio_x") != std::string::npos);
    const std::string restart_factory = between(factory, "void restart_encoder(", "void init_encoders");
    REQUIRE(!restart_factory.empty());
    CHECK(restart_factory.find("pio_sm_put_blocking(pio") == std::string::npos);
    CHECK(restart_factory.find("pio_encode_set(pio_x") != std::string::npos);

    // State-sequencing unit for the sampler logic: sample -> compare with x ->
    // push only on change, with x initialized to the current AB state (the
    // `set x` the CPU forces before enabling the state machine).
    struct Sampler {
        std::uint8_t x;
        std::vector<std::uint8_t> pushes;
        explicit Sampler(std::uint8_t initial) : x(initial) {}
        void sample(std::uint8_t ab) {
            const std::uint8_t y = ab;
            if (x != y) { x = y; pushes.push_back(ab); }
        }
    };
    constexpr std::int8_t delta[16] = {
         0,  1, -1, 127,
        -1,  0, 127,  1,
         1,127,  0, -1,
       127, -1,  1,  0,
    };
    Sampler s(0b11); // current state 11 at init
    s.sample(0b11);
    CHECK(s.pushes.empty()); // no spurious push for the sampled state
    for (int ab : {0b10, 0b00, 0b01, 0b11}) s.sample(static_cast<std::uint8_t>(ab));
    REQUIRE(s.pushes.size() == 4);
    std::int64_t count = 0;
    std::uint8_t last = 0b11;
    for (std::uint8_t ab : s.pushes) { count += delta[(last << 2) | ab]; last = ab; }
    CHECK(count == 4); // full positive Gray sequence, one count per transition
    Sampler illegal(0b00);
    illegal.sample(0b00);
    illegal.sample(0b11); // illegal two-bit jump
    REQUIRE(illegal.pushes.size() == 1);
    CHECK(delta[(0b00 << 2) | 0b11] == 127);
    return 0;
}

// ------------------------------------------------------------------- F07
// docs/01 §4 wire-body sequencing: lead strip before the length feed, the cut,
// then the tail strip with stock withdrawal s2+10 and piece backfeed s2. The
// audit reproduced a 13.949 mm overlap when stock +s1 and piece -s2 moved
// together; this test asserts the bodies never overlap and the sequence is the
// documented one for the audit's 8/6 mm job.
int test_f07_strip_geometry() {
    Rig r; r.boot_and_home();
    Job j = good_job(); // 8 mm lead / 6 mm tail
    REQUIRE(r.load_and_approve(j));
    REQUIRE(r.c.start() == CommandResult::Accepted);

    bool overlap = false;
    bool separation_bad = false;
    bool lead_bad = false;
    int lead_scores = 0, cut_phases = 0, tail_scores = 0, ejects = 0;
    std::int32_t lead_tip_at_score = 0;
    std::int32_t retract_end_tip = 0;
    double cut_l = 0, cut_r = 0;
    std::vector<Phase> trace;
    std::uint32_t ticks = 0;
    Phase prev = Phase::Idle;
    while (r.c.state() == State::Running && ticks < 400000) {
        r.c.tick();
        const Telemetry t = r.c.telemetry();
        const bool entered = t.phase != prev;
        if (t.piece_present && t.stock_tip_um > t.piece_tail_um) overlap = true;
        if (entered) trace.push_back(t.phase);
        if (t.phase == Phase::LeadScore) {
            lead_tip_at_score = t.stock_tip_um;
            if (t.piece_present || t.stock_tip_um != j.strip_lead_um) lead_bad = true;
            if (entered) ++lead_scores;
        }
        if (t.phase == Phase::LeadRetract) retract_end_tip = t.stock_tip_um;
        if (t.phase == Phase::Cut && entered) ++cut_phases;
        if (t.phase == Phase::OpenAfterCut) { cut_l = r.b.feed_true_um[0]; cut_r = r.b.feed_true_um[1]; }
        if (t.phase == Phase::TailScore) {
            if (entered) ++tail_scores;
            // Commanded model positions: stock tip -s2-10, piece tail -s2.
            if (t.piece_tail_um != -j.strip_tail_um ||
                t.piece_tail_um - t.stock_tip_um != 10000) separation_bad = true;
            // Physical simulator positions relative to this cut: the piece tail
            // must stay at least 10 mm to the right of the stock tip.
            const double stock_rel = r.b.feed_true_um[0] - cut_l;
            const double piece_rel = r.b.feed_true_um[1] - cut_r;
            CHECK(stock_rel <= -j.strip_tail_um - 10000 + 150); // withdrawn s2+10
            CHECK(piece_rel <= -j.strip_tail_um + 150);         // backfed s2
            CHECK(piece_rel - stock_rel >= 10000 - 300);
        }
        if (t.phase == Phase::Eject && entered) ++ejects;
        r.b.advance(ctrl::mcu::control_period_us);
        prev = t.phase;
        ++ticks;
    }
    CHECK(r.c.state() == State::Ready);
    CHECK(!overlap);                 // the audit's overlap regression
    CHECK(!separation_bad);          // 10 mm stock/piece separation at the tail score
    CHECK(!lead_bad);
    CHECK(lead_scores == 3);
    CHECK(cut_phases == 4);          // prime sever + one per piece
    CHECK(tail_scores == 3);
    CHECK(ejects == 3);              // the prime stub is swept (waste chute), one ejection per piece
    CHECK(lead_tip_at_score == j.strip_lead_um);
    CHECK(retract_end_tip == -spec::strip_clear_um); // bare tip X-3
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == j.quantity && t.counters.pieces_accepted == j.quantity);

    // Correct ordering inside every piece pass: the lead score precedes the
    // cut and the tail score follows it, with the tail withdrawal in between.
    std::size_t pass = 0;
    auto find_from = [&trace](std::size_t from, Phase p) {
        return static_cast<std::size_t>(std::find(trace.begin() + static_cast<std::ptrdiff_t>(from), trace.end(), p) - trace.begin());
    };
    while (pass < trace.size()) {
        const std::size_t lead = find_from(pass, Phase::LeadAdvance);
        if (lead == trace.size()) break;
        const std::size_t score = find_from(lead, Phase::LeadScore);
        const std::size_t feed = find_from(score, Phase::Feed);
        const std::size_t cut = find_from(feed, Phase::Cut);
        const std::size_t withdraw = find_from(cut, Phase::TailWithdraw);
        const std::size_t tscore = find_from(withdraw, Phase::TailScore);
        CHECK(lead < score && score < feed && feed < cut && cut < withdraw && withdraw < tscore);
        if (!(lead < score && score < feed && feed < cut && cut < withdraw && withdraw < tscore)) break;
        pass = tscore + 1;
    }

    // The wire-body model itself rejects the audited failure classes.
    CHECK(!wire::bodies_valid(100, -100, true));        // stock +s1 / piece -s2 overlap
    CHECK(wire::bodies_valid(0, 0, true));              // contact at the cut plane
    CHECK(wire::bodies_valid(-16000, -6000, true));     // withdrawn separation
    CHECK(wire::bodies_valid(500, -100, false));        // no piece: unrestricted
    CHECK(!wire::lead_score_valid(0, 8000, false));     // stock not advanced: wrong end
    CHECK(!wire::lead_score_valid(8000, 8000, true));   // piece present: wrong end
    CHECK(wire::lead_score_valid(8000, 8000, false));
    CHECK(!wire::tail_score_valid(-10000, -6000, 300000, 8000, 6000, true)); // separation < 10 mm
    CHECK(!wire::tail_score_valid(0, -6000, 300000, 8000, 6000, true));      // stock not withdrawn
    CHECK(!wire::tail_score_valid(-16000, 0, 300000, 8000, 6000, true));     // piece not backfed
    CHECK(wire::tail_score_valid(-16000, -6000, 300000, 8000, 6000, true));
    CHECK(!wire::tail_score_valid(-35000, -25000, 199999, 25000, 25000, true)); // 149.999 mm < 150 mm retained grip
    CHECK(wire::tail_score_valid(-35000, -25000, 200000, 25000, 25000, true));  // exact docs/01 §4 arithmetic
    CHECK(!wire::restore_allowed(true, false));         // stock restore before clearance
    CHECK(wire::restore_allowed(true, true));
    CHECK(wire::restore_allowed(false, false));         // clearance check disabled

    // Independent lead/tail settings run to completion on their own.
    Rig l; l.boot_and_home();
    Job lead_only = good_job(); lead_only.strip_tail_um = 0; lead_only.quantity = 1;
    REQUIRE(l.load_and_approve(lead_only));
    l.press(&Inputs::start, 150);
    REQUIRE(l.run_until(State::Ready, 120000));
    CHECK(l.c.counters().pieces_done == 1 && l.c.counters().pieces_accepted == 1);

    Rig tail_rig; tail_rig.boot_and_home();
    Job tail_only = good_job(); tail_only.strip_lead_um = 0; tail_only.quantity = 1;
    REQUIRE(tail_rig.load_and_approve(tail_only));
    tail_rig.press(&Inputs::start, 150);
    REQUIRE(tail_rig.run_until(State::Ready, 120000));
    CHECK(tail_rig.c.counters().pieces_done == 1 && tail_rig.c.counters().pieces_accepted == 1);
    return 0;
}

// ------------------------------------------------------------------- F08
// strip_travel_um() and validate() must derive the ideal per-jaw closure from
// blade_open_apex/2 - sqrt(2)*r, with the sever overlap kept explicit.
int test_f08_strip_closure() {
    // At an 8 mm open apex the closed-jaw reference is 4.0 mm.
    CHECK(blade_sever_overlap_um == 100); // 0.2 mm total apex overlap at sever
    Job j = good_job();
    // wire 4.0 mm, depth 0.5 mm -> r = 1.5 mm -> closure = 4.0 - sqrt(2)*1.5 = 1.8787 mm
    j.wire_diameter_um = 4000; j.strip_depth_um = 500;
    Rig r; r.run_ms(25);
    REQUIRE(r.c.load_job(j) == CommandResult::Accepted);
    const std::int32_t expected = static_cast<std::int32_t>(std::llround((4.0 - std::sqrt(2.0) * 1.5) * 1000.0));
    CHECK(r.c.strip_travel_um() == expected);
    // The old, incorrect expression used (blade_cut_mm - 0.2) - sqrt(2)*r.
    const std::int32_t old = static_cast<std::int32_t>(std::llround((4.1 - 0.2 - std::sqrt(2.0) * 1.5) * 1000.0));
    CHECK(r.c.strip_travel_um() != old);
    // Exact apex relation: at r = 0 the ideal closure would be the 4.0 mm
    // half-apex; validate() accepts the deepest legal score and rejects a sever.
    Job k = good_job(); k.strip_depth_um = (k.wire_diameter_um / 2) - 100; // r = 0.1 mm
    CHECK(validate(k) == JobError::None);
    k.strip_depth_um = (k.wire_diameter_um + 1) / 2; // depth*2 >= diameter -> sever
    CHECK(validate(k) == JobError::Depth);
    return 0;
}

// ------------------------------------------------------------------- F09
// Motion fault thresholds (1.5 mm gross slip) are separate from piece
// acceptance: after the bounded correction the guarded residual must meet the
// recipe budget (0.20 mm + 0.05 % L), else fault/quarantine and no count.
int test_f09_length_budget() {
    Rig r; r.b.slip_ppm[0] = -2847; r.b.slip_ppm[1] = -2847; // audit repro: 854 um on a 300 mm part
    r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job(300000, 1)));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until_phase(Phase::FeedCorrect, 60000));
    const std::int32_t residual = std::abs(r.c.counters().last_residual_um);
    CHECK(residual >= 800 && residual <= 900); // 854 um initial residual
    // The one bounded correction cannot move the measuring wheel: the frozen
    // feedback leaves the residual outside the 350 um budget after the recheck.
    r.b.encoder_freeze[0] = true; r.b.encoder_freeze[1] = true;
    REQUIRE(r.run_until(State::Fault, 60000));
    CHECK((r.c.faults() & fault::length_budget) != 0);
    CHECK(r.c.counters().pieces_done == 0);
    CHECK(r.c.counters().pieces_accepted == 0); // must not count good
    CHECK(r.c.counters().pieces_scrap >= 1);    // quarantined

    // Strip/pull motions are verified too: a lead advance whose left measuring
    // wheel lags beyond the strip budget faults before any score. Only the
    // stock wheel is disturbed so the (unverified) prime pass is unaffected.
    Rig s; s.b.slip_ppm[0] = -30000; // 3 %: 240 um on the 8 mm lead advance, > 205 um budget
    s.boot_and_home();
    REQUIRE(s.load_and_approve(good_job()));
    s.press(&Inputs::start, 150);
    REQUIRE(s.run_until(State::Fault, 120000));
    CHECK((s.c.faults() & fault::length_budget) != 0);
    CHECK(s.c.counters().pieces_accepted == 0);
    return 0;
}

// ------------------------------------------------------------------- F11
// Explicit good/scrap semantics: PieceDone increments separately from
// accepted-pieces; with the clear check enabled a missing present->clear
// transition must not count good.
int test_f11_part_clear() {
    // Production defaults (M27/M30, handoff 3.2): handling verification is
    // required, cannot be disabled outside service fixture mode, and fixture
    // runs never emit accepted production counts or persistent approval.
    Rig r0; r0.boot_and_home();
    CHECK(r0.c.telemetry().settings.part_clear_check == 1);
    CHECK(r0.c.set_part_clear_check(false) == CommandResult::Unsupported);

    // Natural production cycle on the material plant: the ejected piece
    // presents and clears the discharge sensor inside its physical window.
    Rig r; r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job(300000, 1)));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Ready, 200000));
    CHECK(r.c.counters().pieces_done == 1);
    CHECK(r.c.counters().pieces_accepted == 1);
    CHECK(r.c.counters().pieces_scrap == 0);
    CHECK(r.c.faults() == 0);

    // Early clear: the part presents and clears entirely before PieceDone (fast
    // takeaway). The captured edge history still validates the pair - the old
    // behaviour restarted watching only after ejection and quarantined this.
    Rig r2; r2.boot_and_home();
    REQUIRE(r2.load_and_approve(no_strip_job(300000, 1)));
    r2.b.part_clear_stuck = true; // clear
    r2.press(&Inputs::start, 150);
    bool presented = false;
    for (std::uint32_t i = 0; i < 300000 && r2.c.state() == State::Running; ++i) {
        r2.c.tick(); r2.b.advance(ctrl::mcu::control_period_us);
        if (r2.c.phase() == Phase::Eject && !presented) {
            r2.b.part_clear_stuck = false; r2.run_ms(ctrl::timing::input_debounce_ms + 10); // present
            r2.b.part_clear_stuck = true;  r2.run_ms(ctrl::timing::input_debounce_ms + 10); // cleared early
            presented = true;
        }
    }
    REQUIRE(r2.c.state() == State::Ready);
    CHECK(r2.c.counters().pieces_accepted == 1);

    // Stuck-clear sensor (never shows present): discharge cannot be evidenced
    // -> faulted and quarantined, never accepted.
    Rig r3; r3.boot_and_home();
    REQUIRE(r3.load_and_approve(no_strip_job(300000, 1)));
    r3.b.part_clear_stuck = true;
    r3.press(&Inputs::start, 150);
    REQUIRE(r3.run_until(State::Fault, 300000));
    CHECK(r3.c.faults() & fault::part_not_cleared);
    CHECK(r3.c.counters().pieces_done == 1);
    CHECK(r3.c.counters().pieces_accepted == 0);

    // Stuck-present sensor (never clears): same quarantine.
    Rig r4; r4.boot_and_home();
    REQUIRE(r4.load_and_approve(no_strip_job(300000, 1)));
    r4.b.part_clear_stuck = false;
    r4.press(&Inputs::start, 150);
    REQUIRE(r4.run_until(State::Fault, 300000));
    CHECK(r4.c.faults() & fault::part_not_cleared);
    CHECK(r4.c.counters().pieces_accepted == 0);

    // Fixture mode: handling verification may be disabled for engineering
    // runs, but such runs never count as accepted production output and no
    // persistent approval can exist.
    Rig r5; r5.boot_and_home();
    REQUIRE(r5.c.set_fixture_mode(true) == CommandResult::Accepted);
    CHECK(r5.c.approve() == CommandResult::Unsupported);
    REQUIRE(r5.c.set_part_clear_check(false) == CommandResult::Accepted);
    REQUIRE(r5.c.load_job(no_strip_job(300000, 1)) == CommandResult::Accepted);
    CHECK(r5.c.start() == CommandResult::Accepted); // fixture runs without approval
    REQUIRE(r5.run_until(State::Ready, 200000));
    CHECK(r5.c.counters().pieces_done == 1);
    CHECK(r5.c.counters().pieces_accepted == 0);
    CHECK(r5.c.counters().pieces_fixture == 1);
    // Leaving fixture mode restores the production default.
    REQUIRE(r5.c.set_fixture_mode(false) == CommandResult::Accepted);
    CHECK(r5.c.telemetry().settings.part_clear_check == 1);
    CHECK(!r5.c.telemetry().fixture_mode);
    return 0;
}

// ---------------------------------------------------------- M27/M30 groups
// Sensor observability (M27): station roles are physical. Stock wheels ride
// continuous stock and can never observe a severed piece; the downstream
// station D is the only piece observer and is valid only while the piece
// occupies it; the unobserved ejection remainder needs discharge evidence.
int test_observability_station_roles() {
    // Helper: run until the production piece (not the 10 mm prime stub) is
    // severed and the blades are opening.
    auto run_to_piece_sever = [](RigT<sim::MaterialPlant>& rig) -> bool {
        for (std::uint32_t i = 0; i < 100000; ++i) {
            if (rig.c.state() == State::Fault) return false;
            if (rig.c.phase() == Phase::OpenAfterCut && rig.b.piece_present && rig.b.piece_len_um > 100000.0)
                return true;
            rig.run_ms(1);
        }
        return false;
    };
    // Freeze both stock wheels after the sever: the piece phases must still
    // verify and accept via station D - piece verification never reads the
    // stock wheels (the counterexample defect class).
    Rig r; r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job(300000, 1)));
    r.press(&Inputs::start, 150);
    REQUIRE(run_to_piece_sever(r));
    r.b.encoder_freeze[0] = r.b.encoder_freeze[1] = true;
    REQUIRE(r.run_until(State::Ready, 200000));
    CHECK(r.c.counters().pieces_accepted == 1);
    CHECK(r.c.faults() == 0);

    // Freeze station D after the sever: the piece motion is then physically
    // unobserved and the controller must fault instead of fabricating counts.
    Rig r2; r2.boot_and_home();
    REQUIRE(r2.load_and_approve(no_strip_job(300000, 1)));
    r2.press(&Inputs::start, 150);
    REQUIRE(run_to_piece_sever(r2));
    r2.b.encoder_freeze[2] = true;
    REQUIRE(r2.run_until(State::Fault, 200000));
    CHECK((r2.c.faults() & (fault::length_slip | fault::length_budget | fault::wire_geometry)) != 0);
    CHECK(r2.c.counters().pieces_accepted == 0);

    // The ejection's unobserved remainder is real geometry: for a 300 mm piece
    // at tail X3 the station sees only the first 103.75 mm of the 230 mm move,
    // so discharge evidence is mandatory before acceptance.
    CHECK(Controller::piece_station_observed_um(3000, 300000, 230000) == 103750);
    CHECK(Controller::piece_station_observed_um(3000, 300000, 230000) < 230000);
    // A fully observable piece move (backfeed within occupancy) has no gap.
    CHECK(Controller::piece_station_observed_um(0, 300000, -6000) == 6000);
    return 0;
}

// Waste handling (M30): the prime stub and strip slugs are removed by the
// constrained sweep with a verified chute transit; a trapped body faults
// before any further feed or blade close.
int test_waste_handling() {
    // Blocked sweep: the stub stays at the pocket and the machine faults
    // before feeding over it.
    Rig r; r.boot_and_home();
    r.b.sweep_blocked = true;
    REQUIRE(r.load_and_approve(no_strip_job(300000, 1)));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Fault, 100000));
    CHECK(r.c.faults() & fault::waste_not_cleared);
    CHECK(r.c.counters().pieces_done == 0);
    CHECK(r.b.pocket_waste_bodies() == 1); // never propelled by feed motion

    // Healthy job: exactly one body per release reaches the bin (prime stub
    // once per batch plus the lead and tail slug per stripped piece).
    Rig r2; r2.boot_and_home();
    const Job j = good_job();
    REQUIRE(r2.load_and_approve(j));
    r2.press(&Inputs::start, 150);
    REQUIRE(r2.run_until(State::Ready, 400000));
    CHECK(r2.b.waste_bin_count() == 1 + 2 * j.quantity);
    CHECK(r2.b.pocket_waste_bodies() == 0);
    return 0;
}

// Negative control (M27, handoff 3.1): the legacy fixture couples each encoder
// to its motor's commanded steps - the defect demonstrated by the preserved
// stock-side counterexample. Under the revised observability contract that
// fixture can no longer fabricate a passing machine cycle: with no physical
// piece station and no real handling channels it faults and never emits an
// accepted count. Historical reproduction of the original defect stays pinned
// in tests/reproduction-2026-10-29 against commit 0f28aa3.
int test_negative_control_legacy_fixture() {
    // Part 1: under production handling verification the legacy fixture faults
    // and never accepts - it has no real channels from which to build evidence.
    LegacyRig r; r.boot_and_home();
    REQUIRE(r.load_and_approve(no_strip_job(300000, 1)));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Fault, 100000));
    CHECK(r.c.counters().pieces_accepted == 0);
    CHECK(r.c.counters().pieces_fixture == 0);

    // Part 2: even with handling verification disabled in service fixture mode,
    // the revised observability contract cannot be satisfied by commanded-step
    // couplings: the piece move must be verified at the downstream station,
    // which the legacy fixture does not have, so the run faults instead of
    // fabricating motion. (The old fabricated pass existed only under the old
    // contract; its pinned reproduction is tests/reproduction-2026-10-29.) No
    // path ever relabels fixture completion as accepted output.
    LegacyRig r2; r2.boot_and_home();
    REQUIRE(r2.c.set_fixture_mode(true) == CommandResult::Accepted);
    REQUIRE(r2.c.set_part_clear_check(false) == CommandResult::Accepted);
    REQUIRE(r2.c.load_job(no_strip_job(300000, 1)) == CommandResult::Accepted);
    CHECK(r2.c.start() == CommandResult::Accepted);
    REQUIRE(r2.run_until(State::Fault, 200000));
    CHECK(r2.c.counters().pieces_accepted == 0);
    CHECK(r2.c.counters().pieces_fixture == 0);
    return 0;
}

// Interrupted-run recovery (M37/G43): a stop mid-feed leaves a long in-flight
// stock body. The prime pass must classify it by REAL length (long -> scrap
// discharge with full evidence, uncounted because the interruption already
// counted the scrap) and then continue production normally.
int test_recovery_scrap_eject() {
    Rig r; r.boot_and_home();
    const Job j = no_strip_job(2000000, 5); // 2 m pieces: 2 s of feed at 0.5 m/s
    REQUIRE(r.load_and_approve(j));
    r.press(&Inputs::start, 150);
    // Run until the physical stock tip is deep past the plane (mid piece-1
    // feed), then stop: the interrupt must leave a LONG in-flight body.
    bool mid_feed = false;
    for (std::uint32_t i = 0; i < 200000 && r.c.state() == State::Running; ++i) {
        r.c.tick(); r.b.advance(ctrl::mcu::control_period_us);
        if (r.b.stock_tip_um > 800000.0) { mid_feed = true; break; }
    }
    REQUIRE(mid_feed);
    REQUIRE(r.c.stop() == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Paused, 20000));
    CHECK(r.c.counters().pieces_scrap == 1);
    const double tip_before = r.b.stock_tip_um;
    CHECK(tip_before > 500000.0); // physically mid-feed, not at the plane
    const std::size_t bin_before_resume = r.b.waste_bin_count();
    // Resume: prime severs the long body -> scrap discharge through the
    // takeaway with discharge evidence, then production continues.
    r.press(&Inputs::start, 150);
    bool scrap_ejected = false;
    for (std::uint32_t i = 0; i < 400000 && r.c.state() == State::Running; ++i) {
        r.c.tick(); r.b.advance(ctrl::mcu::control_period_us);
        if (!scrap_ejected && r.c.phase() == Phase::Eject && r.c.counters().pieces_done == 0) {
            scrap_ejected = true; // the prime-severed long body takes the eject path
        }
    }
    REQUIRE(r.c.state() == State::Ready);
    CHECK(scrap_ejected);
    CHECK(r.c.counters().pieces_done == 5);
    CHECK(r.c.counters().pieces_accepted == 5);
    CHECK(r.c.counters().pieces_scrap == 1);        // the interruption only - never double-counted
    CHECK(r.c.counters().pieces_fixture == 0);
    // The long in-flight body was scrap-discharged, never waste-chuted: no
    // additional body may enter the waste bin during the recovery.
    CHECK(r.b.waste_bin_count() == bin_before_resume);
    return 0;
}

// Envelope extremes (G01/G02/G08): minimum length with maximum asymmetric
// strips and maximum length, on the honest plant with production evidence.
int test_envelope_extremes() {
    // L=200 mm with 3/25 asymmetric strips (maximum tail backfeed).
    Rig r; r.boot_and_home();
    Job j{};
    j.length_um = 200000; j.strip_lead_um = 3000; j.strip_tail_um = 25000;
    j.strip_depth_um = 600; j.wire_diameter_um = 2500; j.quantity = 2;
    REQUIRE(r.load_and_approve(j));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Ready, 400000));
    CHECK(r.c.faults() == 0);
    CHECK(r.c.counters().pieces_accepted == 2);
    CHECK(r.b.waste_bin_count() == 1 + 2 * 2); // stub + 2 slugs per stripped piece

    // L=3000 mm no strips: longest support case.
    Rig r2; r2.boot_and_home();
    REQUIRE(r2.load_and_approve(no_strip_job(3000000, 1)));
    r2.press(&Inputs::start, 150);
    REQUIRE(r2.run_until(State::Ready, 400000));
    CHECK(r2.c.faults() == 0);
    CHECK(r2.c.counters().pieces_accepted == 1);
    return 0;
}

// D station origin transfer (M27): across a batch, station D accumulates
// exactly the material that physically passes it - the feed portion past D
// (L - X_D per piece) plus the eject window (X_D - tail0 per piece). For
// unstripped pieces these sum to exactly L per piece: D observes the full
// length of wire that becomes each piece, nothing more. Any motor-coupled
// fabrication (the counterexample defect class) would add impossible counts
// and fail this check.
int test_piece_origin_transfer() {
    Rig r; r.boot_and_home();
    const Job j = no_strip_job(300000, 3);
    REQUIRE(r.load_and_approve(j));
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Ready, 400000));
    const double d_um = ctrl::sensing::piece_station_x_mm * 1000.0;
    const std::int64_t feed_part = static_cast<std::int64_t>(300000.0 - d_um);   // 193250 um
    const std::int64_t eject_part = Controller::piece_station_observed_um(0, 300000, 230000); // 106750 um
    CHECK(feed_part + eject_part == 300000); // exact identity for unstripped pieces
    const std::int64_t expected_counts = 3 * (feed_part + eject_part) * ctrl::enc::nominal_counts_per_metre / 1000000;
    const std::int64_t measured = r.c.telemetry().piece_encoder_counts;
    const std::int64_t delta = measured > expected_counts ? measured - expected_counts : expected_counts - measured;
    CHECK(delta <= 8); // step quantisation and count truncation only
    CHECK(r.c.counters().pieces_accepted == 3);
    return 0;
}

// ------------------------------------------------------------------- F12
// Commissioning/recipe approval interlock keyed to calibration, recipe and
// firmware identity; production start requires it; changes invalidate it.
int test_f12_approval() {
    Rig r; r.boot_and_home();
    Job j = no_strip_job(200000, 1);
    REQUIRE(r.c.load_job(j) == CommandResult::Accepted);
    CHECK(!r.c.telemetry().approved);
    CHECK(r.c.start() == CommandResult::NotApproved); // rejected
    CHECK(r.c.state() == State::Ready);
    REQUIRE(r.c.approve() == CommandResult::Accepted);
    CHECK(r.c.telemetry().approved);
    CHECK(r.c.start() == CommandResult::Accepted);
    CHECK(r.c.state() == State::Running);
    REQUIRE(r.run_until(State::Ready, 120000));
    CHECK(r.c.telemetry().approved); // persists across a completed batch

    // Calibration change invalidates the approval.
    CHECK(r.c.set_calibration(42000, 41600, 41000) == CommandResult::Accepted);
    CHECK(!r.c.telemetry().approved);
    CHECK(r.c.start() == CommandResult::NotApproved);

    // Recipe change invalidates the approval.
    REQUIRE(r.c.approve() == CommandResult::Accepted);
    Job other = j; other.length_um = 250000;
    REQUIRE(r.c.load_job(other) == CommandResult::Accepted);
    CHECK(!r.c.telemetry().approved);
    CHECK(r.c.start() == CommandResult::NotApproved);

    // Approval survives a controller re-init (persistent settings record).
    REQUIRE(r.c.approve() == CommandResult::Accepted);
    {
        Controller c2(r.b);
        c2.init();
        CHECK(c2.telemetry().approved);
        CHECK(c2.telemetry().job_loaded);
    }

    // Firmware identity change: a stored approval keyed to another firmware
    // revision is not valid.
    Settings tampered = r.c.telemetry().settings;
    tampered.approval = Settings::approval_marker;
    tampered.approval_key = commissioning_identity(tampered, "S1-B-OTHER-FIRMWARE");
    tampered.crc = crc16(reinterpret_cast<const std::uint8_t*>(&tampered), offsetof(Settings, crc));
    CHECK(tampered.approval_key != commissioning_identity(tampered));
    REQUIRE(r.b.store_settings(&tampered, sizeof tampered));
    Controller c3(r.b);
    c3.init();
    CHECK(!c3.telemetry().approved);
    // Drive the tampered controller to Ready: production start then refuses.
    for (std::uint32_t i = 0; i < ctrl::timing::input_debounce_ms + 5; ++i) { c3.tick(); r.b.advance(ctrl::mcu::control_period_us); }
    r.b.field.start = true;
    for (std::uint32_t i = 0; i < 150; ++i) { c3.tick(); r.b.advance(ctrl::mcu::control_period_us); }
    r.b.field.start = false;
    for (std::uint32_t i = 0; i < ctrl::timing::homing_timeout_ms + 1000 && c3.state() != State::Ready; ++i) {
        c3.tick(); r.b.advance(ctrl::mcu::control_period_us);
    }
    REQUIRE(c3.state() == State::Ready);
    CHECK(c3.start() == CommandResult::NotApproved);
    return 0;
}

// ------------------------------------------------------------------- F33
// Jog arithmetic must be widened (int64) and checked before any range
// decision: every integer boundary at nonzero blade positions is rejected or
// accepted exactly, with no signed overflow (run under -fsanitize=undefined).
int test_f33_jog_overflow() {
    using lim = std::numeric_limits<std::int32_t>;
    Rig r; r.boot_and_home();
    REQUIRE(r.c.jog(ctrl::axis::blade_front, true, 2000, 5000) == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Ready, 10000));
    CHECK(r.c.telemetry().blade_um[0] == 2000);

    // The audit's UBSan repro: blade_travel_um() + INT_MAX overflowed int32.
    CHECK(r.c.jog(ctrl::axis::blade_front, true, lim::max(), 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, false, lim::max(), 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, true, lim::max() - 2000, 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, true, lim::min(), 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, true, lim::min() + 1, 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.state() == State::Ready);

    // Every integer boundary around the blade travel window at 2 mm.
    CHECK(r.c.jog(ctrl::axis::blade_front, true, 2201, 5000) == CommandResult::InvalidArgument); // 1 past the limit
    CHECK(r.c.jog(ctrl::axis::blade_front, true, 2200, 5000) == CommandResult::Accepted);        // exactly 4.2 mm
    REQUIRE(r.run_until(State::Ready, 10000));
    CHECK(r.c.telemetry().blade_um[0] == 4200);
    CHECK(r.c.jog(ctrl::axis::blade_front, true, 1, 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, false, 4201, 5000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::blade_front, false, 4200, 5000) == CommandResult::Accepted);        // exactly 0
    REQUIRE(r.run_until(State::Ready, 10000));
    CHECK(r.c.telemetry().blade_um[0] == 0);
    CHECK(r.c.jog(ctrl::axis::blade_front, false, lim::max(), 5000) == CommandResult::InvalidArgument);

    // Feed distances: the documented 1 m ceiling and the int32 edges.
    CHECK(r.c.jog(ctrl::axis::feed_left, true, lim::max(), 100000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::feed_left, true, lim::min(), 100000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::feed_left, true, 1000001, 100000) == CommandResult::InvalidArgument);
    CHECK(r.c.jog(ctrl::axis::feed_left, true, 1000000, 100000) == CommandResult::Accepted); // boundary accepted
    CHECK(r.c.stop() == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Ready, 60000));

    // The same boundary through the protocol dispatch path.
    proto::Frame req{}; req.msg = proto::Msg::Jog; req.seq = 3; req.len = proto::jog_len;
    req.payload[0] = static_cast<std::uint8_t>(ctrl::axis::blade_front); req.payload[1] = 1;
    proto::put_i32(req.payload + 2, lim::max()); proto::put_i32(req.payload + 6, 5000);
    proto::Frame reply{};
    proto::dispatch(r.c, req, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::InvalidArgument));
    return 0;
}

// F33 polarity: the per-axis/per-wheel sign multipliers are validated by a
// commissioning move; wrong feedback stores the corrected validated polarity,
// missing feedback is rejected without changing it.
int test_f33_polarity() {
    Rig r; r.run_ms(ctrl::timing::input_debounce_ms + 5);
    CHECK(r.c.state() == State::Unhomed);
    // Physically threaded state: wire routed through both cassettes so the
    // right cassette actually grips stock (with an empty cassette the rollers
    // spin free and no station may invent motion).
    r.b.stock_tip_um = 200000.0;
    r.b.encoder_sign[1] = -1; // right measuring wheel wired inverted
    CHECK(r.c.commission_axis(ctrl::axis::feed_right, true) == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Unhomed, 20000));
    CHECK(r.c.commission_result() == CommandResult::Accepted);
    CHECK(r.c.telemetry().settings.axis_sign[ctrl::axis::feed_right] == -1); // validated correction
    // A logical + jog now yields positive logical feedback.
    const std::int64_t enc_before = r.c.telemetry().encoder_counts[1];
    REQUIRE(r.c.jog(ctrl::axis::feed_right, true, 2000, 50000) == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Unhomed, 20000));
    CHECK(r.c.telemetry().encoder_counts[1] - enc_before > 0);

    // Missing feedback: rejected, stored polarity unchanged.
    r.b.encoder_freeze[1] = true;
    CHECK(r.c.commission_axis(ctrl::axis::feed_right, true) == CommandResult::Accepted);
    REQUIRE(r.run_until(State::Unhomed, 20000));
    CHECK(r.c.commission_result() == CommandResult::InvalidArgument);
    CHECK(r.c.telemetry().settings.axis_sign[ctrl::axis::feed_right] == -1);
    r.b.encoder_freeze[1] = false;

    // Blade axis: no sensor present -> missing feedback, rejected before motion.
    Rig r2; r2.boot_and_home();
    r2.b.blade_true_um[0] = 5000;
    r2.run_ms(ctrl::timing::input_debounce_ms + 5);
    CHECK(r2.c.commission_axis(ctrl::axis::blade_front, true) == CommandResult::InvalidArgument);

    // Blade axis: on the sensor, logical forward leaves it -> validated +1.
    r2.b.blade_true_um[0] = 100;
    r2.run_ms(ctrl::timing::input_debounce_ms + 5);
    CHECK(r2.c.commission_axis(ctrl::axis::blade_front, true) == CommandResult::Accepted);
    REQUIRE(r2.run_until(State::Unhomed, 20000));
    CHECK(r2.c.commission_result() == CommandResult::Accepted);
    CHECK(r2.c.telemetry().settings.axis_sign[ctrl::axis::blade_front] == 1);

    // Blade axis with an inverted stored sign: the move walks toward the
    // sensor, so commissioning stores the corrected +1.
    Rig r3; r3.boot_and_home();
    REQUIRE(r3.c.set_axis_sign(ctrl::axis::blade_front, -1) == CommandResult::Accepted);
    CHECK(r3.c.commission_axis(ctrl::axis::blade_front, true) == CommandResult::Accepted);
    REQUIRE(r3.run_until(State::Unhomed, 20000));
    CHECK(r3.c.commission_result() == CommandResult::Accepted);
    CHECK(r3.c.telemetry().settings.axis_sign[ctrl::axis::blade_front] == 1);
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    const std::string which = argc > 1 ? argv[1] : "all";
    struct { const char* name; int (*fn)(); } tests[] = {
        {"profile", test_profile}, {"validate", test_validate}, {"homing", test_homing}, {"cycle", test_cycle},
        {"correction", test_cycle_no_strip_and_correction}, {"slip", test_slip_fault_and_no_wire},
        {"stop", test_stop_pause_resume}, {"safety", test_safety_and_alarm}, {"board", test_board_fault_latches}, {"jog", test_jog_and_commands},
        {"storage", test_settings_storage_failure}, {"journal", test_settings_journal}, {"protocol", test_protocol},
        {"f05_quadrature_init", test_f05_quadrature_init}, {"f07_strip_geometry", test_f07_strip_geometry},
        {"f08_strip_closure", test_f08_strip_closure}, {"f09_length_budget", test_f09_length_budget},
        {"f11_part_clear", test_f11_part_clear}, {"f12_approval", test_f12_approval},
        {"f33_jog_overflow", test_f33_jog_overflow}, {"f33_polarity", test_f33_polarity},
        {"observability", test_observability_station_roles}, {"waste", test_waste_handling},
        {"negative_control", test_negative_control_legacy_fixture},
        {"recovery", test_recovery_scrap_eject}, {"envelope", test_envelope_extremes},
        {"origin_transfer", test_piece_origin_transfer},
    };
    int rc = 0; bool ran = false;
    for (auto& t : tests) if (which == "all" || which == t.name) { ran = true; rc |= t.fn(); }
    if (!ran) { std::printf("unknown test %s\n", which.c_str()); return 2; }
    if (rc || failures) { std::printf("%d failure(s)\n", failures + rc); return 1; }
    std::printf("ok\n");
    return 0;
}
