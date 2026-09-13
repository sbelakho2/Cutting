#pragma once
#include <cstdint>
#include <cstddef>

namespace starz::fw {
// CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, no final XOR.
// "123456789" -> 0x29B1.
inline std::uint16_t crc16(const std::uint8_t* data, std::size_t len, std::uint16_t crc = 0xFFFF) noexcept {
    for (std::size_t i = 0; i < len; ++i) {
        crc = static_cast<std::uint16_t>(crc ^ (static_cast<std::uint16_t>(data[i]) << 8));
        for (int b = 0; b < 8; ++b)
            crc = static_cast<std::uint16_t>((crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1));
    }
    return crc;
}
}
