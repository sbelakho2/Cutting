// Machine software host tests: step timing exactness, job validation, homing,
// production cycle geometry, encoder verification, stop/pause, fault latching,
// protocol round-trip. Runs with the simulated Board; no hardware involved.
#include "controller.hpp"
#include "crc16.hpp"
#include "protocol.hpp"
#include "settings_journal.hpp"
#include "sim_board.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace starz;
using namespace starz::fw;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)
#define REQUIRE(cond) do { if (!(cond)) { std::printf("FATAL %s:%d: %s\n", __FILE__, __LINE__, #cond); return 1; } } while (0)

namespace {

struct Rig {
    sim::SimBoard b;
    Controller c{b};
    Rig() { c.init(); }
    // Run the 1 kHz loop for ms milliseconds.
    void run_ms(std::uint32_t ms) {
        for (std::uint32_t i = 0; i < ms; ++i) { c.tick(); b.advance(ctrl::mcu::control_period_us); }
    }
    bool run_until(State s, std::uint32_t max_ms) {
        for (std::uint32_t i = 0; i < max_ms; ++i) { if (c.state() == s) return true; run_ms(1); }
        return c.state() == s;
    }
    void press(bool Inputs::*btn, std::uint32_t hold_ms) { b.field.*btn = true; run_ms(hold_ms); b.field.*btn = false; run_ms(ctrl::timing::input_debounce_ms + 5); }
    void boot_and_home() {
        run_ms(ctrl::timing::input_debounce_ms + 5);
        press(&Inputs::start, ctrl::timing::start_hold_ms + ctrl::timing::input_debounce_ms + 10);
        run_until(State::Ready, ctrl::timing::homing_timeout_ms + 1000);
    }
};

Job good_job() {
    Job j{};
    j.length_um = 300000; j.strip_lead_um = 8000; j.strip_tail_um = 6000;
    j.strip_depth_um = 600; j.wire_diameter_um = 2500; j.quantity = 3;
    return j;
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
    CHECK(r.c.load_job(j) == CommandResult::Accepted);
    const double feed0 = r.b.feed_true_um[1];
    r.press(&Inputs::start, 150);
    CHECK(r.c.state() == State::Running);
    REQUIRE(r.run_until(State::Ready, 120000));
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == j.quantity);
    CHECK(t.counters.pieces_scrap == 0);
    CHECK(t.job_loaded); // completed recipe remains available for panel-only repeat batches
    // Blades open at the end, drives still enabled (Ready).
    CHECK(t.blade_um[0] == 0 && t.blade_um[1] == 0);
    // Right feed net advance = prime clearance + N*(length + ejection) with the
    // strip pull/push movements cancelling (each move rounds to a whole step).
    const double expected = spec::stock_clear_um + j.quantity * (static_cast<double>(j.length_um) + ctrl::job::ejection_um);
    CHECK(std::fabs((r.b.feed_true_um[1] - feed0) - expected) < (1 + 2.0 * j.quantity) * 1000.0 / ctrl::axis::feed_steps_per_mm);
    // No pulse violated the driver minimum.
    CHECK(r.b.min_half_period_seen >= min_half_period_ticks);
    // Encoder residual within one count of zero with an ideal roller.
    CHECK(std::abs(t.counters.worst_residual_um) <= static_cast<std::int32_t>(2 * ctrl::enc::nominal_um_per_count));
    CHECK(r.b.stores >= 1); // pieces_total persisted at job end
    return 0;
}

int test_cycle_no_strip_and_correction() {
    Rig r; r.b.slip_ppm[0] = -3000; r.b.slip_ppm[1] = -3000; // roller reads 0.3 % short of command
    r.boot_and_home();
    Job j = good_job(); j.strip_lead_um = 0; j.strip_tail_um = 0; j.strip_depth_um = 0; j.quantity = 2; j.length_um = 300000; // 0.9 mm short, < max_slip
    REQUIRE(r.c.load_job(j) == CommandResult::Accepted);
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Ready, 120000));
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == 2);
    // One corrective move brings the encoder-measured length within the deadband.
    CHECK(std::abs(t.counters.last_residual_um) <= static_cast<std::int32_t>(2 * ctrl::enc::nominal_um_per_count));
    return 0;
}

int test_slip_fault_and_no_wire() {
    Rig r; r.b.slip_ppm[0] = -20000; // 2 % short -> 6 mm on 300 mm > 1.5 mm
    r.boot_and_home();
    REQUIRE(r.c.load_job(good_job()) == CommandResult::Accepted);
    r.press(&Inputs::start, 150);
    REQUIRE(r.run_until(State::Fault, 60000));
    CHECK(r.c.faults() & fault::length_slip);
    CHECK(r.c.telemetry().job_loaded); // job retained across the fault
    CHECK(r.b.outputs.drives_disabled);

    Rig r2; r2.b.wire_present = false;
    r2.boot_and_home();
    REQUIRE(r2.c.load_job(good_job()) == CommandResult::Accepted);
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
    Job disagreement = good_job(); disagreement.quantity = 1; disagreement.strip_lead_um = 0;
    disagreement.strip_tail_um = 0; disagreement.strip_depth_um = 0;
    REQUIRE(r3.c.load_job(disagreement) == CommandResult::Accepted);
    r3.press(&Inputs::start, 150);
    REQUIRE(r3.run_until(State::Fault, 60000));
    CHECK(r3.c.faults() & fault::length_slip);
    return 0;
}

int test_stop_pause_resume() {
    Rig r; r.boot_and_home();
    Job j = good_job(); j.quantity = 5; j.length_um = 2000000;
    REQUIRE(r.c.load_job(j) == CommandResult::Accepted);
    r.press(&Inputs::start, 150);
    r.run_ms(2000); // mid-feed of the first production piece or prime
    CHECK(r.c.state() == State::Running);
    r.b.field.stop_ok = false; // NC STOP pressed
    r.run_ms(50);
    CHECK(r.c.state() == State::Stopping || r.c.state() == State::Paused);
    REQUIRE(r.run_until(State::Paused, 5000));
    CHECK(!r.b.outputs.drives_disabled); // controlled stop keeps holding torque
    // START ignored while STOP still pressed.
    r.press(&Inputs::start, 150);
    CHECK(r.c.state() == State::Paused);
    r.b.field.stop_ok = true; r.run_ms(50);
    r.press(&Inputs::start, 150);
    CHECK(r.c.state() == State::Running);
    REQUIRE(r.run_until(State::Ready, 200000));
    const Telemetry t = r.c.telemetry();
    CHECK(t.counters.pieces_done == 5);
    CHECK(t.counters.pieces_scrap == 1);
    return 0;
}

int test_safety_and_alarm() {
    Rig r; r.boot_and_home();
    REQUIRE(r.c.load_job(good_job()) == CommandResult::Accepted);
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
    CHECK(r.c.set_calibration(41000, 40900) == CommandResult::Accepted);
    CHECK(r.c.set_calibration(30000, 40900) == CommandResult::InvalidArgument);
    const Job saved = good_job();
    CHECK(r.c.load_job(saved) == CommandResult::Accepted);
    // Calibration and the valid screenless operating recipe survive a re-init.
    Controller c2(r.b); c2.init();
    CHECK(c2.telemetry().settings.counts_per_metre[0] == 41000);
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
    CHECK(r.c.set_calibration(41000, 40900) == CommandResult::StorageError);
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
    malformed_status[1] = static_cast<std::uint8_t>(Phase::Stop) + 1;
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[4] |= 0x40; // reserved input bit
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[6] |= 0x10; // reserved output bit
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    malformed_status[7] = 2; // job_loaded must be canonical
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    std::memcpy(malformed_status, reply.payload, sizeof malformed_status);
    proto::put_u32(malformed_status + 96, 0); // loaded recipe cannot have zero quantity
    CHECK(!proto::decode_status(malformed_status, proto::status_len, t));
    Telemetry wide{};
    wide.feed_um[0] = 3000000000ll; wide.feed_um[1] = -3000000000ll;
    wide.encoder_counts[0] = 3000000000ll; wide.encoder_counts[1] = -3000000000ll;
    std::uint8_t status[proto::status_len]{};
    proto::encode_status(wide, status);
    Telemetry decoded{};
    REQUIRE(proto::decode_status(status, proto::status_len, decoded));
    CHECK(decoded.feed_um[0] == wide.feed_um[0] && decoded.feed_um[1] == wide.feed_um[1]);
    CHECK(decoded.encoder_counts[0] == wide.encoder_counts[0] && decoded.encoder_counts[1] == wide.encoder_counts[1]);
    proto::Frame malformed{}; malformed.msg = proto::Msg::Start; malformed.seq = 8; malformed.len = 1; malformed.payload[0] = 0;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::MalformedFrame));
    malformed = proto::Frame{}; malformed.msg = proto::Msg::Jog; malformed.seq = 9; malformed.len = proto::jog_len;
    malformed.payload[0] = 0; malformed.payload[1] = 2;
    proto::dispatch(r.c, malformed, reply);
    CHECK(reply.msg == proto::Msg::Nack && reply.payload[0] == static_cast<std::uint8_t>(CommandResult::InvalidArgument));
    // Wire-level CRC check vector.
    CHECK(crc16(reinterpret_cast<const std::uint8_t*>("123456789"), 9) == 0x29B1);
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
    };
    int rc = 0; bool ran = false;
    for (auto& t : tests) if (which == "all" || which == t.name) { ran = true; rc |= t.fn(); }
    if (!ran) { std::printf("unknown test %s\n", which.c_str()); return 2; }
    if (rc || failures) { std::printf("%d failure(s)\n", failures + rc); return 1; }
    std::printf("ok\n");
    return 0;
}
