// STARZ S1-B RP2350 firmware entry point.
// The physical machine is intentionally screenless. START, STOP, RESET/ACK,
// E-stop and the hardwired safety circuit are sufficient for operation; this
// optional USB-CDC link is only a bounded remote job/status interface.
#include "board_rp2350.hpp"
#include "controller.hpp"
#include "protocol.hpp"

#include "pico/stdlib.h"
#include "pico/time.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "tusb.h"
#pragma GCC diagnostic pop

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace starz::fw::rp2350 {
namespace {

class UsbCdcLink {
public:
    void service(Controller& controller, std::uint64_t now_us) {
        // A host flood cannot starve pulse FIFO service: consume at most 128
        // bytes and transmit at most 64 queued bytes per foreground iteration.
        decoder_.expire(now_us);
        tud_task();
        for (std::uint32_t count = 0; count < max_rx_per_service && tud_cdc_available(); ++count) {
            std::uint8_t byte = 0;
            if (tud_cdc_read(&byte, 1) != 1) break;
            if (decoder_.feed(byte, now_us)) {
                proto::Frame reply{};
                proto::dispatch(controller, decoder_.frame(), reply);
                enqueue(reply);
            }
        }
        flush();
    }

private:
    static constexpr std::uint32_t partial_frame_timeout_us = 100000;
    static constexpr std::uint32_t max_rx_per_service = 128;
    static constexpr std::uint32_t max_tx_per_service = 64;
    static constexpr std::uint32_t ring_size = 256;

    bool enqueue(const proto::Frame& frame) {
        std::uint8_t encoded[proto::max_frame]{};
        const std::size_t count = proto::encode(frame, encoded, sizeof(encoded));
        if (count == 0 || count > ring_size - used_) return false;
        for (std::size_t i = 0; i < count; ++i) {
            tx_[head_] = encoded[i];
            head_ = (head_ + 1u) % ring_size;
            ++used_;
        }
        return true;
    }

    void flush() {
        if (!tud_cdc_connected() || used_ == 0) return;
        const std::uint32_t contiguous = head_ >= tail_ ? head_ - tail_ : ring_size - tail_;
        const std::uint32_t count = static_cast<std::uint32_t>(
            contiguous < used_ ? contiguous : used_);
        const std::uint32_t available = tud_cdc_write_available();
        const std::uint32_t send = std::min({count, available, max_tx_per_service});
        if (send == 0) return;
        const std::uint32_t sent = tud_cdc_write(tx_ + tail_, send);
        tail_ = (tail_ + sent) % ring_size;
        used_ -= sent;
        tud_cdc_write_flush();
    }

    proto::TimedDecoder decoder_{partial_frame_timeout_us};
    std::uint8_t tx_[ring_size]{};
    std::uint32_t head_ = 0;
    std::uint32_t tail_ = 0;
    std::uint32_t used_ = 0;
};

} // namespace
} // namespace starz::fw::rp2350

int main() {
    using namespace starz::fw;
    using namespace starz::fw::rp2350;

    BoardRp2350 board;
    board.initialize();
    Controller controller{board};
    controller.init();
    UsbCdcLink usb;

    std::uint64_t next_tick_us = board.now_us();
    for (;;) {
        board.service();
        usb.service(controller, board.now_us());
        controller.pump();

        const std::uint64_t now = board.now_us();
        if (now >= next_tick_us) {
            controller.tick();
            next_tick_us += starz::ctrl::mcu::control_period_us;
            // Do not run a backlog of stale supervisory ticks after a debugger
            // halt or unexpectedly long ISR. Resume the fixed 1 kHz cadence.
            if (now >= next_tick_us)
                next_tick_us = now + starz::ctrl::mcu::control_period_us;
        }
        tight_loop_contents();
    }
}
