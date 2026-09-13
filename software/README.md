# STARZ software status

**No software in this directory authorises fabrication, energisation, machine operation, or a personnel-safety claim.** Passing a host test or producing a UF2 establishes neither physical stop time, safe standstill, guard-release permission, production accuracy, endurance, equipment reliability, nor accepted-part quality.

## Current source boundaries

| Path | Scope | Boundary |
| --- | --- | --- |
| [firmware/](firmware/) | Legacy host-side C++ fault-injection reference controller and its Qt reference monitor | It is not target firmware and deliberately rejects production job execution. |
| [machine/core/](machine/core/) | Host-testable production-controller logic: motion planning, sequencing, retained recipe, protocol and safety supervision | `SimBoard` tests exercise logic only; no real machine or safety function has been validated. |
| [machine/rp2350/](machine/rp2350/) | Raspberry Pi Pico 2 / RP2350 board adapter, PIO sources, production `starz-machine` target and separate `if-b01-factory-test` fixture target | Requires the Pico SDK and has not been exercised on physical hardware. The fixture image is not production firmware. |
| [machine/host/](machine/host/) | `starz-ctl` USB-CDC commissioning/service CLI for retained recipes, calibration, jogging and telemetry | The screenless machine must not depend on the CLI for normal production operation. USB is not a safety function. |

The root CMake `core` and `sanitize` presets compile the host-testable code; `sanitize` enables AddressSanitizer and UndefinedBehaviorSanitizer checks. The Pico SDK project in [machine/rp2350/](machine/rp2350/) is intentionally configured separately. The cabinet/safety design input is [docs/22-controls-and-wiring.md](../docs/22-controls-and-wiring.md); it remains unreleased and its safety boundary is hardwired, not software-defined.

## Legacy reference controls only

**NOT FOR MACHINE OPERATION.** The remainder of this document describes only the bounded host-side reference implementation in [firmware/](firmware/). It is **not RP2350 firmware**, a PIO implementation, a commissioned process controller, a hardware watchdog, or a personnel-safety system. Previous descriptions implying that the legacy reference has those capabilities or any safety certification are withdrawn.

## Build boundary

- The historical `starz_firmware_core` target name is retained for source
  compatibility; `starz_reference_controls` is its descriptive CMake alias.
- Configuration explicitly rejects `STARZ_BUILD_HARDWARE=ON`, cross-compilation
  and Pico SDK/platform configurations. Public reference headers require
  `STARZ_REFERENCE_ONLY=1`; Pico device macros are rejected too. These guards
  prevent accidental use, not a malicious modification of the source.
- `pio_step_dir` and `pio_quadrature` are quarantined legacy **counter test
  doubles**, excluded from the core and unused by the controller. The optional
  `STARZ_BUILD_LEGACY_TEST_DOUBLES` target requires a separate explicit test
  macro. It produces no pins, pulses, quadrature decoding or physical outputs.
- The Qt HMI is a reference monitor. All motion, home, process, calibration,
  pneumatic and guard commands are disabled both in the UI and at the serial
  command gate. Only heartbeat and software abort requests may be transmitted.
  Abort is **not an emergency-stop function**. Do not connect it to a machine.

## Reference controller contract

- Startup is `FAULT`, with unknown samples, unknown home references and no
  heartbeat. Positions use `INT32_MIN` for unknown, never a guessed home at zero.
  Hardware outputs are always absent/disabled. The default `MonitorOnly`
  controller rejects all jogs. Only explicitly selected `FaultInjection` mode
  can execute a mathematical reference trajectory.
- Input and per-axis feedback samples are copied through explicitly named
  `inject_*_for_test` seams. A valid sample is a **fixture observation**, not
  proof of a real sensor. The controller never generates encoder measurements
  from its commands. Tests may inject ideal feedback or independently hold,
  remove or perturb it. No physical axis plant is claimed.
- Every sample must be marked valid; all four feedback axes require an explicit
  known measured reference. Safety/feedback observations expire at **50,000 us**.
  These are unqualified reference-test limits, not approved machine limits.
  The caller must provide actual sample deliveries, not resubmit cached values
  and describe them as new measurements.
- E-stop/relay loss, open guard, driver alarm, wire absence, slug jam and invalid
  samples trip on their **level in the sample call itself**. No debounce, rising
  edge or delayed trip qualification exists. Commanded guard lock/unlock never
  edits the measured guard feedback. Unlock requests are denied without a fresh,
  independently supplied safe-permission observation; idle/zero software speed
  cannot produce that permission. Even with it, only a test request bit exists.
- The communication monitor trips at **exactly 750,000 us** since the last new,
  valid, zero-payload `CMD_PING`. Other commands, malformed frames, CRC failures
  and rejected duplicate pings do not refresh it. All ages saturate without
  arithmetic wrap. This is caller-clocked software, **not a hardware watchdog**:
  a stopped process or stopped caller cannot be detected by the same process.
- Fault bits are sticky. Good inputs, new heartbeat, abort, resume and repeated
  `init()` cannot clear a trip. Deliberate `trigger_recovery()` / `CMD_RECOVER`
  requires fresh healthy inputs, heartbeat, explicit measured references on all
  axes, no unlock request and no active reference motion. Recovery returns to
  idle only. It discards old moves, requires a new command and never restarts.
  Clearing a bit does not reset physical hardware or establish safe standstill.
- Abort discards the reference move and invalidates its reference; fresh feedback
  and deliberate recovery are needed before another move. Resumable pause is not
  modeled: pause conservatively aborts and returns `NotCommissioned`. Resume,
  blind homing and full process jobs are rejected. Valid production jobs always
  return **`NotCommissioned`**, including in fault-injection mode. Invalid fields
  return explicit validation failures before any state mutation.
- All job/jog fields are validated at the controller boundary, not only in the
  HMI. Lengths, speed, direction, axis, strips, blade depth and quantity have
  bounds; combined distances/targets use widened checked arithmetic. Strip zero
  means no strip; otherwise the shared minimum/maximum applies. Blade strip
  depth must be positive and below the sever position. No material suitability
  or process accuracy is inferred from these numeric bounds.

## Trajectory and feedback limitations

Four independent axes (feed L/R, blade F/R) evaluate analytic triangular or
trapezoidal profiles from integer accumulated microseconds. This avoids
tick-dependent Euler integration, accumulated integer-step truncation and the
old forced creep minimum. Endpoints, speed, acceleration and duration are
bounded. Reference travel is ±3 m for feed and 0–4.2 mm for blades using the
current shared specification. These are fixture coordinate bounds, not measured
machine envelopes. Floating-point math is deterministic for the same build and
elapsed time; cross-platform bit-identical real-time pulse timing is not claimed.

Fresh injected feedback is monitored independently from the reference command.
Following-error test thresholds are 1,000 um for feed and 100 um for blades.
Arrival requires exact integer-fixture position matching after the profile ends;
a stuck small move that stays within the error band still times out at 30 s.
Larger following errors trip immediately on observation. These thresholds are
not safety or production tolerances. A drive/encoder that reports plausible but
incorrect data cannot be diagnosed by this model. Recovered fixture coordinates
are deliberately re-established from new explicit observations, not from the
aborted command.

There is no cutting/stripping/clamping/ejection/material/contact-force model,
sensor-verified process sequence, hardware scheduling, production calibration,
or quality acceptance logic. **Completed parts, target parts and cycle time stay
zero.** Finishing a reference axis profile does not fabricate a finished part.
No throughput, physical stop-time or certification claim can be derived from
these boolean/unit tests. Independent safety hardware, qualified interlocks,
real sensing, process commissioning and physical validation remain mandatory
before any separate machine implementation could be considered.

## Legacy binary protocol

Frame: `A5 5A | message:u8 | sequence:u8 | length:u8 | payload | CRC16`.
Payload length is at most **64 bytes**, so the maximum frame is 71 bytes.
SOF and CRC are sent most-significant-byte first. **All multibyte payload fields
are little endian**, signed values use two's complement. Explicit codecs are
used instead of copying native packed structs onto the wire. CRC-16/CCITT-FALSE
uses polynomial `0x1021`, initial `0xFFFF`, no reflection or final XOR
(`123456789` gives `0x29B1`). CRC is corruption detection, not authentication.

| Payload | Exact bytes | Layout (payload offsets) |
| --- | ---: | --- |
| Job start | 20 | length:i32@0, lead:i32@4, tail:i32@8, depth:i32@12, quantity:u32@16 |
| Jog | 10 | axis:u8@0, direction:i8@1, speed:i32@2, distance:i32@6 |
| Telemetry | **30** | state:u8@0, flags:u8@1, feed:i32@2, injected encoder:i32@6, front:i32@10, rear:i32@14, completed:u32@18, target:u32@22, cycle:u16@26, fault bits:u16@28 |
| Ping / pause / resume / abort / recover / home / guard | 0 | No payload, no extra bytes allowed |
| Air / calibration (rejected, uncommissioned) | 2 / 4 | duration:u16 / scale:i32 |
| ACK / NACK | 1 | `CommandResult` value, echoed request sequence |
| PONG | 0 | Echoed ping sequence |

`CMD_RECOVER` is `0x14`. Result values are defined by `CommandResult`: accepted,
invalid length/argument, unsafe state, not commissioned, unknown home, range,
busy, replay, unsupported, permission denied and malformed frame. The controller
provides a **one-response mailbox**, not an unbounded queue or serial server;
the fixture must drain it between commands if every response is needed.

Telemetry flags bits 0–5 are sampled relay, guard, wire, slug, driver alarm and
latched E-stop respectively; bit 6 marks a fresh valid input sample; bit 7 marks
this **reference-only protocol**. Unobserved/stale positive input bits are not
displayed as healthy. Fault bits 0–11 represent unknown inputs, stale inputs,
E-stop, guard, driver, wire, slug, heartbeat, unknown feedback, stale feedback,
following error and move timeout. Zero fault bits are not a safety approval.
The legacy 30-byte format does not carry feed-R or all measured-axis telemetry;
the C++ test API exposes separate reference commands. Extending real telemetry
would require an explicitly versioned contract.

Frames use fixed-size storage, enforce exact command payload sizes and expire
partial frames at 100,000 us. Invalid frames have no command/heartbeat effect.
The sequence window rejects duplicate/backward/ambiguous (distance ≥128) sequence
numbers; wrap from 255 to 0 is supported. Exact valid abort is always honored,
even if duplicated/out of order. Reinitializing a live controller preserves the
sequence window. **This is not secure anti-replay**: 8-bit wrap, a new process,
reconnection, adversarial traffic and identity/session changes are not protected.
There is no authentication, encryption or commissioned transport/session layer.

## Legacy HMI freshness

The prototype warning is permanent. The monitor accepts only exact 30-byte,
CRC-checked, sequence-checked telemetry with known states, the reference marker,
bounded positions and zero unmodeled production counters. No telemetry is
considered present on connection. Disconnect, malformed/replayed data and
750 ms without fresh telemetry clear status/KPIs and mark positions unknown.
Freshness is checked against a monotonic clock; the UI refresh timer is 50 ms,
so visible invalidation can be later if the Qt event loop stalls. Motion remains
disabled regardless. Progress uses widened, clamped arithmetic; display deltas
are widened before subtraction. No assumed production-rate multiplier remains.

## Legacy tests and build configurations

The repository's `core` and `sanitize` configure/build/test presets include these
legacy host reference tests as well as the host-testable machine core. A separate `build-controls-check` configuration with
`STARZ_BUILD_HMI=ON` and `CMAKE_BUILD_TYPE=Release` also
builds the Qt monitor and its offscreen test. The optional legacy-double target
can be compiled separately with `STARZ_BUILD_LEGACY_TEST_DOUBLES=ON`.

Tests use an always-active throwing `CHECK`, never `assert`, and self-test that a
failed check is detected. Coverage includes unknown startup, sampled input loss,
immediate level E-stop/guard/alarm trips, sticky fault recovery/no restart, exact
750,000 us heartbeat expiry, stale/missing/stuck feedback, small-move timeout,
four independent analytic trajectories, invalid homing, numeric extremes,
malformed/truncated/noisy frames, exact binary lengths, known CRC and endian
vectors, serialization lengths 0–255, limited replay filtering, rejected jobs,
zero invented accepted counts, stale HMI state and permanently disabled controls.
The offscreen HMI test does not open a serial device. No Python, GPIO, physical
motion, force/temperature measurement or hardware qualification is involved.