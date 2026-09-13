#include "pio_quadrature.hpp"
#include <limits>
#include "../protocol/starz_protocol.hpp"

namespace starz::firmware {

QuadratureDecoder::QuadratureDecoder(uint8_t /*pin_a*/, uint8_t /*pin_b*/) {}

void QuadratureDecoder::init() {
    raw_counts_ = 0;
    scale_nm_per_count_ = 0;
    counts_valid_ = false;
}

bool QuadratureDecoder::set_scale_nm_per_count(int32_t scale_nm) {
    scale_nm_per_count_ = scale_nm > 0 && scale_nm <= 1000000 ? scale_nm : 0;
    return scale_nm_per_count_ != 0;
}

void QuadratureDecoder::reset_count() {
    raw_counts_ = 0;
    counts_valid_ = false;
}

bool QuadratureDecoder::update_mock_edges(int32_t delta_counts) {
    const int64_t counts = static_cast<int64_t>(raw_counts_) + delta_counts;
    if (counts < std::numeric_limits<int32_t>::min() || counts > std::numeric_limits<int32_t>::max()) {
        counts_valid_ = false;
        return false;
    }
    raw_counts_ = static_cast<int32_t>(counts);
    counts_valid_ = true;
    return true;
}

int32_t QuadratureDecoder::get_measured_um() const {
    if (!counts_valid_ || scale_nm_per_count_ <= 0) return protocol::UNKNOWN_POSITION_UM;
    const int64_t measured = static_cast<int64_t>(raw_counts_) * scale_nm_per_count_ / 1000;
    if (measured <= std::numeric_limits<int32_t>::min() || measured > std::numeric_limits<int32_t>::max()) {
        return protocol::UNKNOWN_POSITION_UM;
    }
    return static_cast<int32_t>(measured);
}

} // namespace starz::firmware
