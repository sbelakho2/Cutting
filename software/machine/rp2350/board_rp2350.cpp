#include "board_rp2350.hpp"
#include "crc16.hpp"
#include "profile.hpp"
#include "step_gen.pio.h"
#include "quadrature.pio.h"

#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "pico/error.h"
#include "pico/flash.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include <cstddef>
#include <cstring>
#include <limits>

namespace starz::fw::rp2350 {
namespace {

constexpr std::uint8_t step_pio_index[ctrl::axis::count] = {0, 0, 1, 1};
constexpr std::uint8_t step_sm_index[ctrl::axis::count] = {0, 1, 0, 1};
constexpr std::uint8_t step_pul_pin[ctrl::axis::count] = {
    ctrl::pin::blade_front_pul, ctrl::pin::blade_rear_pul,
    ctrl::pin::feed_left_pul, ctrl::pin::feed_right_pul,
};
constexpr std::uint8_t step_dir_pin[ctrl::axis::count] = {
    ctrl::pin::blade_front_dir, ctrl::pin::blade_rear_dir,
    ctrl::pin::feed_left_dir, ctrl::pin::feed_right_dir,
};
constexpr std::uint8_t encoder_sm_index[2] = {2, 3};
constexpr std::uint8_t encoder_a_pin[2] = {ctrl::pin::enc_left_a, ctrl::pin::enc_right_a};
constexpr std::uint32_t step_pio_mask[2] = {(1u << 0) | (1u << 1), (1u << 0) | (1u << 1)};
constexpr std::uint32_t settings_bytes = BoardRp2350::settings_slots * FLASH_SECTOR_SIZE;
constexpr std::uint32_t settings_offset = BoardRp2350::physical_flash_bytes - settings_bytes;

static_assert(sizeof(BoardRp2350::FlashRecord) <= FLASH_PAGE_SIZE);
static_assert(settings_offset % FLASH_SECTOR_SIZE == 0);
static_assert((ctrl::mcu::sys_clock_hz / ctrl::axis::pio_clock_hz) * ctrl::axis::pio_clock_hz == ctrl::mcu::sys_clock_hz);

bool active_low(std::uint8_t pin) {
    return gpio_get(pin) == 0;
}

std::uint8_t ab_at(std::uint8_t pin_a) {
    return static_cast<std::uint8_t>((gpio_get(pin_a) ? 1u : 0u) | (gpio_get(pin_a + 1u) ? 2u : 0u));
}

// Index = prior AB state * 4 + present AB state. Positive is the explicitly
// wired sequence A/B: 00 -> 01 -> 11 -> 10 -> 00.
constexpr std::int8_t quadrature_delta[16] = {
     0,  1, -1, 127,
    -1,  0, 127,  1,
     1,127,  0, -1,
   127, -1,  1,  0,
};

struct FlashWrite {
    std::uint32_t offset;
    alignas(FLASH_PAGE_SIZE) std::uint8_t record_page[FLASH_PAGE_SIZE];
    alignas(FLASH_PAGE_SIZE) std::uint8_t commit_page[FLASH_PAGE_SIZE];
};

// The record data and its validity latch deliberately occupy separate flash
// pages. A power interruption can therefore leave a partial record page, but
// cannot mark it committed; the previous A/B slot remains the valid record.
void __not_in_flash_func(write_settings_pages)(void* raw) {
    auto* write = static_cast<FlashWrite*>(raw);
    flash_range_erase(write->offset, FLASH_SECTOR_SIZE);
    flash_range_program(write->offset, write->record_page, FLASH_PAGE_SIZE);
    flash_range_program(write->offset + FLASH_PAGE_SIZE, write->commit_page, FLASH_PAGE_SIZE);
}

} // namespace

BoardRp2350* BoardRp2350::instance_ = nullptr;

BoardRp2350::BoardRp2350() = default;

void BoardRp2350::initialize() {
    instance_ = this;
    if (!set_sys_clock_khz(ctrl::mcu::sys_clock_hz / 1000u, false))
        boot_faults_ |= fault::internal;

    stdio_init_all();

    // Inputs supplied by dry relay contacts; every field-contact input is
    // normally pulled high and is logically asserted while held at 0 V.
    const std::uint8_t contact_inputs[] = {
        ctrl::pin::in_start, ctrl::pin::in_stop_ok, ctrl::pin::in_reset,
        ctrl::pin::in_safety_ok, ctrl::pin::in_home_front, ctrl::pin::in_home_rear,
    };
    for (std::uint8_t pin : contact_inputs) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
    const std::uint8_t alarm_inputs[] = {
        ctrl::pin::in_alm_blade_front, ctrl::pin::in_alm_blade_rear,
        ctrl::pin::in_alm_feed_left, ctrl::pin::in_alm_feed_right,
    };
    for (std::uint8_t pin : alarm_inputs) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }

    // IF-B01 holds this active-high enable net low with 100 kOhm. Put the safe
    // low latch in place before enabling GP17; an MCU reset/watchdog recovery
    // therefore cannot enable a drive.
    gpio_init(ctrl::pin::out_drv_enable);
    gpio_put(ctrl::pin::out_drv_enable, 0);
    gpio_set_dir(ctrl::pin::out_drv_enable, GPIO_OUT);
    const std::uint8_t outputs[] = {
        ctrl::pin::out_led_green, ctrl::pin::out_led_red, ctrl::pin::out_buzzer,
    };
    for (std::uint8_t pin : outputs) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
    }

    step_offset_[0] = static_cast<std::uint16_t>(pio_add_program(pio0, &step_gen_program));
    step_offset_[1] = static_cast<std::uint16_t>(pio_add_program(pio1, &step_gen_program));
    for (std::uint8_t i = 0; i < ctrl::axis::count; ++i) {
        StepSm& sm = step_[i];
        sm.pio = step_pio_index[i] == 0 ? pio0 : pio1;
        sm.sm = step_sm_index[i];
        sm.pul_pin = step_pul_pin[i];
        sm.dir_pin = step_dir_pin[i];
        sm.program_offset = step_offset_[step_pio_index[i]];
        sm.busy_until_us = 0;
        gpio_init(sm.dir_pin);
        gpio_set_dir(sm.dir_pin, GPIO_OUT);
        gpio_put(sm.dir_pin, 0);
        step_gen_program_init(sm.pio, sm.sm, sm.program_offset, sm.pul_pin, ctrl::axis::pio_clock_hz);
        pio_sm_set_pins_with_mask(sm.pio, sm.sm, 0, 1u << sm.pul_pin);
    }

    quadrature_offset_ = static_cast<std::uint16_t>(pio_add_program(pio0, &quadrature_program));
    for (std::uint8_t side = 0; side < 2; ++side) {
        encoder_last_[side] = ab_at(encoder_a_pin[side]);
        quadrature_program_init(pio0, encoder_sm_index[side], quadrature_offset_, encoder_a_pin[side]);
    }

    if (watchdog_caused_reboot())
        boot_faults_ |= fault::watchdog_reset;
    watchdog_enable(ctrl::mcu::watchdog_timeout_ms, true);

    // A safety-loop opening is handled without waiting for the 1 kHz core tick.
    gpio_set_irq_enabled_with_callback(ctrl::pin::in_safety_ok, GPIO_IRQ_EDGE_RISE, true, gpio_irq);
    initialized_ = true;
}

std::uint64_t BoardRp2350::now_us() {
    return time_us_64();
}

Inputs BoardRp2350::read_inputs() {
    Inputs in{};
    in.start = active_low(ctrl::pin::in_start);
    in.stop_ok = active_low(ctrl::pin::in_stop_ok);
    in.reset = active_low(ctrl::pin::in_reset);
    in.safety_ok = field_safety_ok();
    in.home_front = active_low(ctrl::pin::in_home_front);
    in.home_rear = active_low(ctrl::pin::in_home_rear);
    // Discharge-clearance and waste-chute channels (ctrl::sensing) are defined
    // in the observability contract but not yet assigned to physical pins: the
    // assignment is the Phase-H I/O-architecture gate (all 26 exposed GPIOs are
    // allocated; see requirements/signal-table.md). Unassigned channels FAIL
    // CLOSED - both read "not clear" - so handling acceptance can never be
    // faked by absent hardware and no waste transit can be claimed. Production
    // handling verification does not pass until these channels are implemented.
    in.part_clear = false;
    in.waste_clear = false;
    in.alm[0] = active_low(ctrl::pin::in_alm_blade_front);
    in.alm[1] = active_low(ctrl::pin::in_alm_blade_rear);
    in.alm[2] = active_low(ctrl::pin::in_alm_feed_left);
    in.alm[3] = active_low(ctrl::pin::in_alm_feed_right);
    return in;
}

void BoardRp2350::write_outputs(const Outputs& outputs) {
    const bool force_disabled = latched_faults_.load(std::memory_order_relaxed) != 0 || !field_safety_ok();
    gpio_put(ctrl::pin::out_drv_enable, (outputs.drives_disabled || force_disabled) ? 0 : 1);
    gpio_put(ctrl::pin::out_led_green, outputs.led_green ? 1 : 0);
    gpio_put(ctrl::pin::out_led_red, outputs.led_red ? 1 : 0);
    gpio_put(ctrl::pin::out_buzzer, outputs.buzzer ? 1 : 0);
    // Outputs::waste_sweep drives the constrained mechanical sweep actuator.
    // Its pin/relay belongs to the Phase-H I/O architecture (see
    // requirements/signal-table.md). Until assigned it is intentionally not
    // driven: the waste verification then faults closed instead of pretending
    // the sweep ran.
    (void)outputs.waste_sweep;
}

void BoardRp2350::set_direction(Axis axis, bool forward) {
    const StepSm& sm = step_[axis];
    gpio_put(sm.dir_pin, forward ? 1 : 0);
    // The next controller call can only queue after this call returns. The PIO
    // run begins after FIFO/clock latency, well beyond the specified 2 us setup.
    busy_wait_us_32(ctrl::axis::dir_setup_us);
}

std::uint32_t BoardRp2350::step_fifo_level(Axis axis) {
    const StepSm& sm = step_[axis];
    return pio_sm_get_tx_fifo_level(sm.pio, sm.sm);
}

void BoardRp2350::push_steps(Axis axis, const StepRun& run) {
    if (run.count == 0 || run.count > 65536u || run.half_period_ticks == 0 || run.half_period_ticks > 65536u) {
        latch_fault(fault::internal);
        return;
    }
    if (latched_faults_.load(std::memory_order_relaxed) != 0) return;
    StepSm& sm = step_[axis];
    const std::uint32_t word = ((run.count - 1u) << 16) | (run.half_period_ticks - 1u);
    pio_sm_put(sm.pio, sm.sm, word);
    const std::uint64_t now = now_us();
    const std::uint64_t start = sm.busy_until_us > now ? sm.busy_until_us : now;
    const std::uint64_t pulse_ticks = 2ull * run.half_period_ticks + pio_period_overhead_ticks;
    const std::uint64_t duration_us = (static_cast<std::uint64_t>(run.count) * pulse_ticks * 1000000ull +
                                       ctrl::axis::pio_clock_hz - 1u) / ctrl::axis::pio_clock_hz;
    // Account for the final PIO instruction boundary and timer read granularity.
    sm.busy_until_us = start + duration_us + 2u;
}

bool BoardRp2350::steps_idle(Axis axis) {
    const StepSm& sm = step_[axis];
    return pio_sm_is_tx_fifo_empty(sm.pio, sm.sm) && now_us() >= sm.busy_until_us;
}

void BoardRp2350::restart_step_sm(StepSm& sm, bool enable) {
    pio_sm_set_enabled(sm.pio, sm.sm, false);
    pio_sm_clear_fifos(sm.pio, sm.sm);
    pio_sm_restart(sm.pio, sm.sm);
    pio_sm_clkdiv_restart(sm.pio, sm.sm);
    pio_sm_set_pins_with_mask(sm.pio, sm.sm, 0, 1u << sm.pul_pin);
    pio_sm_exec(sm.pio, sm.sm, pio_encode_jmp(sm.program_offset));
    sm.busy_until_us = now_us();
    if (enable) pio_sm_set_enabled(sm.pio, sm.sm, true);
}

void BoardRp2350::abort_steps(Axis axis) {
    StepSm& sm = step_[axis];
    restart_step_sm(sm, latched_faults_.load(std::memory_order_relaxed) == 0);
}

std::int64_t BoardRp2350::encoder_count(std::uint8_t side) {
    return encoder_count_[side];
}

bool BoardRp2350::encoder_healthy(std::uint8_t side) {
    return encoder_ok_[side];
}

FaultBits BoardRp2350::boot_faults() {
    return boot_faults_;
}

FaultBits BoardRp2350::latched_faults() {
    return latched_faults_.load(std::memory_order_relaxed);
}

bool BoardRp2350::clear_latched_faults() {
    if (!field_safety_ok()) return false;
    latched_faults_.store(0, std::memory_order_relaxed);
    for (std::uint8_t side = 0; side < 2; ++side) {
        restart_encoder_sm(side);
    }
    for (StepSm& sm : step_) {
        const bool enable = field_safety_ok() && latched_faults_.load(std::memory_order_relaxed) == 0;
        restart_step_sm(sm, enable);
        if (!enable) return false;
    }
    if (!field_safety_ok() || latched_faults_.load(std::memory_order_relaxed) != 0) {
        for (StepSm& sm : step_) restart_step_sm(sm, false);
        return false;
    }
    return true;
}

const BoardRp2350::FlashRecord* BoardRp2350::flash_slot(std::uint32_t slot) {
    const auto address = XIP_BASE + settings_offset + slot * FLASH_SECTOR_SIZE;
    return reinterpret_cast<const FlashRecord*>(address);
}

const std::uint32_t* BoardRp2350::flash_commit_slot(std::uint32_t slot) {
    const auto address = XIP_BASE + settings_offset + slot * FLASH_SECTOR_SIZE + FLASH_PAGE_SIZE;
    return reinterpret_cast<const std::uint32_t*>(address);
}

void BoardRp2350::restart_encoder_sm(std::uint8_t side) {
    const std::uint8_t sm = encoder_sm_index[side];
    const std::uint32_t debug_bit = 1u << (PIO_FDEBUG_RXSTALL_LSB + sm);
    pio_sm_set_enabled(pio0, sm, false);
    pio_sm_clear_fifos(pio0, sm);
    pio_sm_restart(pio0, sm);
    pio_sm_clkdiv_restart(pio0, sm);
    pio0->fdebug = debug_bit; // clear a prior blocking PUSH/RX FIFO stall
    pio_sm_exec(pio0, sm, pio_encode_jmp(quadrature_offset_));
    encoder_last_[side] = ab_at(encoder_a_pin[side]);
    // Load the sampled AB state straight into X. The RX-joined FIFO has no TX
    // side, so pio_sm_put_blocking() here would stall forever.
    pio_sm_exec(pio0, sm, pio_encode_set(pio_x, encoder_last_[side]));
    encoder_ok_[side] = true;
    pio_sm_set_enabled(pio0, sm, true);
}
bool BoardRp2350::load_settings(void* dst, std::uint32_t len) {
    if (len != sizeof(Settings)) return false;
    FlashRecord records[settings_slots]{};
    std::uint32_t commit_latches[settings_slots]{};
    for (std::uint32_t slot = 0; slot < settings_slots; ++slot) {
        records[slot] = *flash_slot(slot);
        commit_latches[slot] = *flash_commit_slot(slot);
    }
    const std::int32_t selected = journal::select(records, commit_latches, settings_slots);
    if (selected < 0) return false;
    const FlashRecord& record = records[static_cast<std::uint32_t>(selected)];
    std::memcpy(dst, &record.settings, sizeof(Settings));
    active_slot_ = selected;
    generation_ = record.generation;
    return true;
}

bool BoardRp2350::store_settings(const void* src, std::uint32_t len) {
    if (len != sizeof(Settings) || !initialized_) return false;
    FlashRecord record{};
    record.magic = flash_magic;
    record.generation = generation_ + 1u;
    std::memcpy(&record.settings, src, sizeof(Settings));
    record.record_crc = journal::record_crc(record);

    FlashWrite write{};
    write.offset = settings_offset + static_cast<std::uint32_t>(active_slot_ == 0 ? 1 : 0) * FLASH_SECTOR_SIZE;
    std::memset(write.record_page, 0xff, sizeof(write.record_page));
    std::memset(write.commit_page, 0xff, sizeof(write.commit_page));
    std::memcpy(write.record_page, &record, sizeof(record));
    std::memcpy(write.commit_page, &flash_commit, sizeof(flash_commit));
    if (flash_safe_execute(write_settings_pages, &write, ctrl::mcu::watchdog_timeout_ms) != PICO_OK) return false;

    const std::uint32_t selected = active_slot_ == 0 ? 1u : 0u;
    if (!journal::valid(*flash_slot(selected), *flash_commit_slot(selected))) return false;
    active_slot_ = static_cast<std::int32_t>(selected);
    generation_ = record.generation;
    return true;
}

void BoardRp2350::watchdog_feed() {
    watchdog_update();
}

bool BoardRp2350::field_safety_ok() const {
    return active_low(ctrl::pin::in_safety_ok);
}

void BoardRp2350::latch_fault(FaultBits reason) {
    emergency_disable(reason);
}

void __not_in_flash_func(BoardRp2350::emergency_disable)(FaultBits reason) {
    latched_faults_.fetch_or(reason, std::memory_order_relaxed);
    pio_set_sm_mask_enabled(pio0, step_pio_mask[0], false);
    pio_set_sm_mask_enabled(pio1, step_pio_mask[1], false);
    for (StepSm& sm : step_) {
        pio_sm_clear_fifos(sm.pio, sm.sm);
        pio_sm_set_pins_with_mask(sm.pio, sm.sm, 0, 1u << sm.pul_pin);
        sm.busy_until_us = time_us_64();
    }
    gpio_put(ctrl::pin::out_drv_enable, 0);
}

void __not_in_flash_func(BoardRp2350::gpio_irq)(uint gpio, std::uint32_t events) {
    if (gpio == ctrl::pin::in_safety_ok && (events & GPIO_IRQ_EDGE_RISE) != 0 && instance_ != nullptr)
        instance_->emergency_disable(fault::safety_open);
}

void BoardRp2350::service_encoder(std::uint8_t side) {
    const std::uint8_t sm = encoder_sm_index[side];
    const std::uint32_t debug_bit = 1u << (PIO_FDEBUG_RXSTALL_LSB + sm);
    if ((pio0->fdebug & debug_bit) != 0) {
        pio0->fdebug = debug_bit;
        encoder_ok_[side] = false;
        latch_fault(fault::encoder_overflow);
        return;
    }
    while (!pio_sm_is_rx_fifo_empty(pio0, sm)) {
        const std::uint8_t current = static_cast<std::uint8_t>(pio_sm_get(pio0, sm) & 0x3u);
        const std::int8_t delta = quadrature_delta[(encoder_last_[side] << 2) | current];
        encoder_last_[side] = current;
        if (delta == 127) {
            encoder_ok_[side] = false;
            latch_fault(fault::encoder_invalid);
            return;
        }
        encoder_count_[side] += delta;
    }
}

void BoardRp2350::service() {
    if (!initialized_) return;
    service_encoder(0);
    service_encoder(1);
}

} // namespace starz::fw::rp2350
