#pragma once

#include <array>
#include <cstddef>
#include <string_view>

// Controlled S1-B cabinet/IF-B01 integration allocation. This declares
// reference designations and controller-visible interfaces only; it does not
// select terminal hardware, wire sizes, protective devices or qualify a safety
// function. See docs/22-controls-and-wiring.md for the design boundary.
namespace starz::cabinet {

inline constexpr std::string_view revision = "S1-B-CTRL-CAB-R2-2026-09-13";
inline constexpr bool if_b01_is_a_safety_function = false;
inline constexpr bool guard_unlock_release_is_authorized = false;
inline constexpr std::size_t field_relay_count = 10;

struct ContactInput {
    std::string_view relay;
    std::string_view board_pin;
    std::string_view return_pin;
    std::string_view controller_gpio;
    std::string_view asserted_meaning;
};

inline constexpr std::array<ContactInput, field_relay_count> contact_inputs{{
    {"KA1", "J10-1", "J10-2", "GP12", "START pressed"},
    {"KA2", "J10-3", "J10-4", "GP13", "STOP loop healthy"},
    {"KA3", "J10-5", "J10-6", "GP14", "RESET/ACK pressed"},
    {"KA4", "J10-7", "J10-8", "GP15", "XPS diagnostic energised"},
    {"KA5", "J10-9", "J10-10", "GP16", "rear home"},
    {"KA6", "J10-11", "J10-12", "GP27", "front home"},
    {"KA7", "J11-1", "J11-2", "GP18", "blade-front drive alarm"},
    {"KA8", "J11-3", "J11-4", "GP19", "blade-rear drive alarm"},
    {"KA9", "J11-5", "J11-6", "GP20", "feed-left drive alarm"},
    {"KA10", "J11-7", "J11-8", "GP21", "feed-right drive alarm"},
}};

struct PowerTerminal {
    std::string_view designation;
    std::string_view net;
    std::string_view board_connection;
    std::string_view status;
};

inline constexpr std::array<PowerTerminal, 6> power_terminals{{
    {"TB24-01", "+24V_PELV", "J3-1", "branch protection and terminal product open"},
    {"TB24-02", "0V_PELV", "J3-2/J3-4/J12-4", "return-current design open"},
    {"TB24-03", "+5V_LOGIC", "J3-3", "branch protection and terminal product open"},
    {"TB-DL", "+24V_DRVLOGIC", "drive PUL+/DIR+/ENA+ only", "fuse, terminals and current calculation open"},
    {"TB-M1", "+60V_BF", "K1 to blade-front drive", "DC interruption, fusing and discharge open"},
    {"TB-M2/TB-M3", "+60V_BR/+36V_FEED", "K2/K3 to remaining drives", "DC interruption, fusing and discharge open"},
}};

struct IsolationPath {
    std::string_view contactor;
    std::string_view source;
    std::string_view safety_output;
    std::string_view proof_status;
};

inline constexpr std::array<IsolationPath, 3> proposed_isolation_paths{{
    {"K1", "LE-350-60 blade-front", "XPS 13-14", "DC opening duty not qualified"},
    {"K2", "LE-350-60 blade-rear", "XPS 23-24", "DC opening duty not qualified"},
    {"K3", "LE-350-36 feed", "XPS 33-34", "DC opening duty not qualified"},
}};

static_assert(contact_inputs.size() == field_relay_count);
static_assert(proposed_isolation_paths.size() == 3);

}  // namespace starz::cabinet