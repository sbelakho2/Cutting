// IF-B01 factory-test firmware for a bare interface board on its dedicated
// fixture. It is intentionally a separate UF2 from production firmware: it
// never starts motion PIO state machines and starts with every TPL input low.
#include "controls_spec.hpp"
#include "quadrature.pio.h"

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/regs/pio.h"
#include "hardware/watchdog.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "tusb.h"
#pragma GCC diagnostic pop

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace {

struct OutputBinding {
    std::uint8_t pin;
    const char* name;
};

constexpr OutputBinding output_bindings[] = {
    {starz::ctrl::pin::blade_front_pul, "U2.CH1 BF_PUL"},
    {starz::ctrl::pin::blade_front_dir, "U2.CH2 BF_DIR"},
    {starz::ctrl::pin::blade_rear_pul, "U2.CH3 BR_PUL"},
    {starz::ctrl::pin::blade_rear_dir, "U2.CH4 BR_DIR"},
    {starz::ctrl::pin::feed_left_pul, "U2.CH5 FL_PUL"},
    {starz::ctrl::pin::feed_left_dir, "U2.CH6 FL_DIR"},
    {starz::ctrl::pin::feed_right_pul, "U2.CH7 FR_PUL"},
    {starz::ctrl::pin::feed_right_dir, "U3.CH1 FR_DIR"},
    {starz::ctrl::pin::out_drv_enable, "U3.CH2 ENA"},
    {starz::ctrl::pin::out_led_green, "U3.CH3 GREEN"},
    {starz::ctrl::pin::out_led_red, "U3.CH4 RED"},
    {starz::ctrl::pin::out_buzzer, "U3.CH5 BUZZER"},
};

struct InputBinding {
    std::uint8_t pin;
    const char* name;
};

constexpr InputBinding input_bindings[] = {
    {starz::ctrl::pin::in_start, "START"},
    {starz::ctrl::pin::in_stop_ok, "STOP_OK"},
    {starz::ctrl::pin::in_reset, "RESET"},
    {starz::ctrl::pin::in_safety_ok, "SAFETY_OK"},
    {starz::ctrl::pin::in_home_rear, "HOME_REAR"},
    {starz::ctrl::pin::in_alm_blade_front, "ALM_BF"},
    {starz::ctrl::pin::in_alm_blade_rear, "ALM_BR"},
    {starz::ctrl::pin::in_alm_feed_left, "ALM_FL"},
    {starz::ctrl::pin::in_alm_feed_right, "ALM_FR"},
    {starz::ctrl::pin::in_home_front, "HOME_FRONT"},
};

constexpr std::size_t output_count = sizeof(output_bindings) / sizeof(output_bindings[0]);
constexpr std::size_t input_count = sizeof(input_bindings) / sizeof(input_bindings[0]);
constexpr std::uint64_t arm_window_us = 30'000'000ull;
constexpr std::uint64_t output_window_us = 3'000'000ull;
constexpr std::int8_t quadrature_delta[16] = {
     0,  1, -1, 127,
    -1,  0, 127,  1,
     1,127,  0, -1,
   127, -1,  1,  0,
};

static_assert(output_count == 12);
static_assert(input_count == 10);

struct EncoderCapture {
    std::uint8_t a_pin;
    std::uint8_t sm;
    std::uint8_t last_ab = 0;
    std::int64_t count = 0;
    bool healthy = true;
};

EncoderCapture encoders[] = {
    {starz::ctrl::pin::enc_left_a, 0},
    {starz::ctrl::pin::enc_right_a, 1},
};

std::uint8_t encoder_program_offset = 0;
bool armed = false;
std::uint64_t armed_until_us = 0;
std::int32_t active_output = -1;
std::uint64_t output_started_us = 0;
volatile bool safety_interrupt = false;
char command_line[80]{};
std::size_t command_length = 0;

bool active_low(std::uint8_t pin) {
    return gpio_get(pin) == 0;
}

std::uint8_t read_ab(std::uint8_t pin_a) {
    return static_cast<std::uint8_t>((gpio_get(pin_a) ? 1u : 0u) |
                                     (gpio_get(static_cast<std::uint8_t>(pin_a + 1u)) ? 2u : 0u));
}

void all_outputs_off() {
    for (const OutputBinding& output : output_bindings) gpio_put(output.pin, 0);
    active_output = -1;
    output_started_us = 0;
}

void __not_in_flash_func(safety_irq)(uint gpio, std::uint32_t events) {
    if (gpio == starz::ctrl::pin::in_safety_ok && (events & GPIO_IRQ_EDGE_RISE) != 0) {
        all_outputs_off();
        safety_interrupt = true;
    }
}

bool safety_and_reset_held() {
    return active_low(starz::ctrl::pin::in_safety_ok) && active_low(starz::ctrl::pin::in_reset);
}

bool output_permitted(std::uint64_t now_us, bool report) {
    if (!armed) {
        if (report) std::printf("ERR not armed\n");
        return false;
    }
    if (safety_interrupt || now_us >= armed_until_us || !safety_and_reset_held()) {
        all_outputs_off();
        armed = false;
        if (report) std::printf("ERR safety, RESET hold, or arm timeout\n");
        return false;
    }
    return true;
}

void supervise_outputs(std::uint64_t now_us) {
    if (safety_interrupt) {
        armed = false;
        safety_interrupt = false;
        std::printf("SAFE safety-loop opening\n");
        return;
    }
    if (armed && !output_permitted(now_us, false)) {
        std::printf("SAFE safety, RESET hold, or arm timeout\n");
        return;
    }
    if (active_output >= 0 && now_us - output_started_us >= output_window_us) {
        all_outputs_off();
        std::printf("SAFE output timeout\n");
    }
}

void init_inputs() {
    for (const InputBinding& input : input_bindings) {
        gpio_init(input.pin);
        gpio_set_dir(input.pin, GPIO_IN);
        gpio_pull_up(input.pin);
    }
}

void init_outputs() {
    for (const OutputBinding& output : output_bindings) {
        gpio_init(output.pin);
        gpio_put(output.pin, 0);
        gpio_set_dir(output.pin, GPIO_OUT);
    }
}

void restart_encoder(EncoderCapture& encoder) {
    const std::uint32_t debug_bit = 1u << (PIO_FDEBUG_RXSTALL_LSB + encoder.sm);
    pio_sm_set_enabled(pio0, encoder.sm, false);
    pio_sm_clear_fifos(pio0, encoder.sm);
    pio_sm_restart(pio0, encoder.sm);
    pio_sm_clkdiv_restart(pio0, encoder.sm);
    pio0->fdebug = debug_bit;
    pio_sm_exec(pio0, encoder.sm, pio_encode_jmp(encoder_program_offset));
    encoder.last_ab = read_ab(encoder.a_pin);
    // RX-joined FIFO: initialize the sampler X register with a forced set
    // instruction; a TX put would block because the TX side is joined away.
    pio_sm_exec(pio0, encoder.sm, pio_encode_set(pio_x, encoder.last_ab));
    encoder.count = 0;
    encoder.healthy = true;
    pio_sm_set_enabled(pio0, encoder.sm, true);
}

void init_encoders() {
    encoder_program_offset = static_cast<std::uint8_t>(pio_add_program(pio0, &quadrature_program));
    for (EncoderCapture& encoder : encoders) {
        quadrature_program_init(pio0, encoder.sm, encoder_program_offset, encoder.a_pin);
        encoder.last_ab = read_ab(encoder.a_pin);
    }
}

void service_encoder(EncoderCapture& encoder) {
    const std::uint32_t debug_bit = 1u << (PIO_FDEBUG_RXSTALL_LSB + encoder.sm);
    if ((pio0->fdebug & debug_bit) != 0) {
        pio0->fdebug = debug_bit;
        encoder.healthy = false;
    }
    while (!pio_sm_is_rx_fifo_empty(pio0, encoder.sm)) {
        const std::uint8_t current = static_cast<std::uint8_t>(pio_sm_get(pio0, encoder.sm) & 0x3u);
        const std::size_t delta_index = (static_cast<std::size_t>(encoder.last_ab) << 2u) | current;
        const std::int8_t delta = quadrature_delta[delta_index];
        encoder.last_ab = current;
        if (delta == 127) encoder.healthy = false;
        else encoder.count += delta;
    }
}

void service_encoders() {
    for (EncoderCapture& encoder : encoders) service_encoder(encoder);
}

char* next_token(char*& cursor) {
    while (*cursor != '\0' && std::isspace(static_cast<unsigned char>(*cursor))) ++cursor;
    if (*cursor == '\0') return nullptr;
    char* start = cursor;
    while (*cursor != '\0' && !std::isspace(static_cast<unsigned char>(*cursor))) ++cursor;
    if (*cursor != '\0') *cursor++ = '\0';
    return start;
}

bool parse_u32(const char* text, std::uint32_t& value) {
    if (text == nullptr || *text == '\0' || *text == '-' || *text == '+') return false;
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if (end == text || *end != '\0' || parsed > static_cast<unsigned long>(std::numeric_limits<std::uint32_t>::max())) return false;
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

bool no_more_tokens(char* cursor) {
    return next_token(cursor) == nullptr;
}

void print_inputs() {
    for (const InputBinding& input : input_bindings)
        std::printf("IN %-11s GP%u asserted=%u raw=%u\n", input.name, static_cast<unsigned>(input.pin),
                    active_low(input.pin) ? 1u : 0u, gpio_get(input.pin) ? 1u : 0u);
}

void print_encoders() {
    constexpr const char* side[] = {"LEFT", "RIGHT"};
    for (std::size_t i = 0; i < sizeof(encoders) / sizeof(encoders[0]); ++i) {
        const EncoderCapture& encoder = encoders[i];
        std::printf("ENC %s count=%lld ab=%u healthy=%u\n", side[i], static_cast<long long>(encoder.count),
                    static_cast<unsigned>(encoder.last_ab), encoder.healthy ? 1u : 0u);
    }
}

void print_state() {
    const char* active_name = active_output >= 0 ? output_bindings[static_cast<std::size_t>(active_output)].name : "none";
    std::printf("STATE armed=%u safety_ok=%u reset=%u active=%s\n", armed ? 1u : 0u,
                active_low(starz::ctrl::pin::in_safety_ok) ? 1u : 0u,
                active_low(starz::ctrl::pin::in_reset) ? 1u : 0u, active_name);
}

void print_help() {
    std::printf("IF-B01 factory test commands:\n");
    std::printf("  IDENT | STATE | INPUTS | ENC | ENC RESET | SAFE\n");
    std::printf("  ARM (requires SAFETY_OK and RESET held asserted)\n");
    std::printf("  OUT <0-11> <0|1> (one output, maximum 3 s on)\n");
    std::printf("  WALK <50-2000 ms> (all 12 outputs, requires ARM and held RESET)\n");
}

void arm_outputs() {
    all_outputs_off();
    if (!safety_and_reset_held()) {
        std::printf("ERR ARM requires asserted SAFETY_OK and held RESET\n");
        return;
    }
    safety_interrupt = false;
    armed = true;
    armed_until_us = time_us_64() + arm_window_us;
    std::printf("OK armed for 30 s; motor connectors J4-J7 must be unplugged\n");
}

void set_output(std::uint32_t index, std::uint32_t level) {
    if (index >= output_count || level > 1u) {
        std::printf("ERR OUT arguments\n");
        return;
    }
    if (level == 0u) {
        all_outputs_off();
        std::printf("OK SAFE\n");
        return;
    }
    if (!output_permitted(time_us_64(), true)) return;
    all_outputs_off();
    const OutputBinding& output = output_bindings[index];
    gpio_put(output.pin, 1);
    active_output = static_cast<std::int32_t>(index);
    output_started_us = time_us_64();
    std::printf("OUT %u %s on\n", static_cast<unsigned>(index), output.name);
}

void walk_outputs(std::uint32_t dwell_ms) {
    if (dwell_ms < 50u || dwell_ms > 2000u || !output_permitted(time_us_64(), true)) {
        if (dwell_ms < 50u || dwell_ms > 2000u) std::printf("ERR WALK dwell must be 50..2000 ms\n");
        return;
    }
    for (std::size_t i = 0; i < output_count; ++i) {
        if (!output_permitted(time_us_64(), true)) {
            std::printf("ERR WALK interrupted\n");
            return;
        }
        all_outputs_off();
        gpio_put(output_bindings[i].pin, 1);
        active_output = static_cast<std::int32_t>(i);
        output_started_us = time_us_64();
        std::printf("WALK %u %s on\n", static_cast<unsigned>(i), output_bindings[i].name);
        for (std::uint32_t elapsed_ms = 0; elapsed_ms < dwell_ms; ++elapsed_ms) {
            tud_task();
            watchdog_update();
            if (!output_permitted(time_us_64(), false)) {
                std::printf("ERR WALK interrupted\n");
                return;
            }
            sleep_ms(1);
        }
    }
    all_outputs_off();
    std::printf("OK WALK complete; outputs safe\n");
}

void handle_command(char* line) {
    char* cursor = line;
    char* command = next_token(cursor);
    if (command == nullptr) return;
    if (std::strcmp(command, "HELP") == 0 && no_more_tokens(cursor)) {
        print_help();
    } else if (std::strcmp(command, "IDENT") == 0 && no_more_tokens(cursor)) {
        std::printf("IF-B01-FACTORY-TEST R1; production protocol untouched\n");
    } else if (std::strcmp(command, "STATE") == 0 && no_more_tokens(cursor)) {
        print_state();
    } else if (std::strcmp(command, "INPUTS") == 0 && no_more_tokens(cursor)) {
        print_inputs();
    } else if (std::strcmp(command, "ENC") == 0) {
        char* action = next_token(cursor);
        if (action == nullptr) print_encoders();
        else if (std::strcmp(action, "RESET") == 0 && no_more_tokens(cursor)) {
            for (EncoderCapture& encoder : encoders) restart_encoder(encoder);
            std::printf("OK ENC reset\n");
        } else {
            std::printf("ERR ENC arguments\n");
        }
    } else if (std::strcmp(command, "SAFE") == 0 && no_more_tokens(cursor)) {
        armed = false;
        all_outputs_off();
        std::printf("OK SAFE\n");
    } else if (std::strcmp(command, "ARM") == 0 && no_more_tokens(cursor)) {
        arm_outputs();
    } else if (std::strcmp(command, "OUT") == 0) {
        char* index_text = next_token(cursor);
        char* level_text = next_token(cursor);
        std::uint32_t index = 0;
        std::uint32_t level = 0;
        if (parse_u32(index_text, index) && parse_u32(level_text, level) && no_more_tokens(cursor)) set_output(index, level);
        else std::printf("ERR OUT arguments\n");
    } else if (std::strcmp(command, "WALK") == 0) {
        char* dwell_text = next_token(cursor);
        std::uint32_t dwell_ms = 0;
        if (parse_u32(dwell_text, dwell_ms) && no_more_tokens(cursor)) walk_outputs(dwell_ms);
        else std::printf("ERR WALK arguments\n");
    } else {
        std::printf("ERR unknown command; use HELP\n");
    }
}

void service_usb_console() {
    tud_task();
    for (;;) {
        const int received = getchar_timeout_us(0);
        if (received < 0) return;
        const char c = static_cast<char>(received);
        if (c == '\r' || c == '\n') {
            if (command_length != 0u) {
                command_line[command_length] = '\0';
                handle_command(command_line);
                command_length = 0;
            }
        } else if (std::isprint(static_cast<unsigned char>(c)) != 0 && command_length + 1u < sizeof(command_line)) {
            command_line[command_length++] = c;
        } else {
            command_length = 0;
            std::printf("ERR command too long or invalid\n");
        }
    }
}

} // namespace

int main() {
    stdio_init_all();
    init_inputs();
    init_outputs();
    init_encoders();
    gpio_set_irq_enabled_with_callback(starz::ctrl::pin::in_safety_ok, GPIO_IRQ_EDGE_RISE, true, safety_irq);
    watchdog_enable(starz::ctrl::mcu::watchdog_timeout_ms, true);
    std::printf("IF-B01 factory test R1 ready; all outputs are low. Type HELP.\n");

    for (;;) {
        supervise_outputs(time_us_64());
        service_encoders();
        service_usb_console();
        watchdog_update();
        tight_loop_contents();
    }
}