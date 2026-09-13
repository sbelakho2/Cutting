#pragma once
// Hardware abstraction implemented by rp2350/board.cpp on the target and by
// tests/sim_board.hpp on the host. The core never touches registers.
#include <cstdint>
#include "types.hpp"

namespace starz::fw {

class Board {
public:
    virtual ~Board() = default;

    virtual std::uint64_t now_us() = 0;
    virtual Inputs read_inputs() = 0;          // raw levels already converted to logical booleans.
    virtual void write_outputs(const Outputs& o) = 0;

    // Step generation. Direction is latched before the first run of a move and
    // must not change while runs are queued.
    virtual void set_direction(Axis a, bool forward) = 0;
    virtual std::uint32_t step_fifo_level(Axis a) = 0; // queued runs, 0..step_fifo_depth
    virtual void push_steps(Axis a, const StepRun& run) = 0;
    virtual bool steps_idle(Axis a) = 0;       // FIFO empty and state machine waiting.
    virtual void abort_steps(Axis a) = 0;      // flush FIFO, stop pulses now (drives keep position).
    static constexpr std::uint32_t step_fifo_depth = 8;

    // Measuring encoders, x4 decoded, signed, free-running.
    virtual std::int64_t encoder_count(std::uint8_t side) = 0; // 0 = left, 1 = right
    // False means the board lost encoder transitions (for example a PIO RX FIFO
    // overflow). The controller latches an encoder-overflow fault instead of
    // trusting an incomplete length measurement.
    virtual bool encoder_healthy(std::uint8_t side) { (void)side; return true; }

    // Board-specific causes that must leave a reset controller faulted. The
    // RP2350 board reports a watchdog reset here; host simulations default to 0.
    virtual std::uint16_t boot_faults() { return 0; }

    // Asynchronous board faults. The RP2350 implementation latches an input
    // safety-loop edge in its GPIO ISR and PIO RX FIFO overflow here, so a
    // transient cannot be missed between 1 kHz controller samples.
    virtual std::uint16_t latched_faults() { return 0; }
    // Called only after the core has confirmed healthy field inputs during an
    // explicit RESET acknowledgement. Returns false if hardware cannot clear.
    virtual bool clear_latched_faults() { return true; }

    // Persistent settings (calibration, lifetime count, and the valid retained
    // production recipe) in flash. A failed store must leave the old record
    // intact so the core can retain its prior in-RAM state.
    virtual bool load_settings(void* dst, std::uint32_t len) = 0;
    virtual bool store_settings(const void* src, std::uint32_t len) = 0;

    virtual void watchdog_feed() = 0;
};

} // namespace starz::fw
