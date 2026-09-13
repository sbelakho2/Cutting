#pragma once
#include <cstdint>
#include "legacy_test_only.hpp"

namespace starz::firmware {

// Legacy injected-count test double. NO encoder acquisition, quadrature or PIO.
class QuadratureDecoder {
public:
    QuadratureDecoder(uint8_t pin_a, uint8_t pin_b);

    void init();
    bool set_scale_nm_per_count(int32_t scale_nm);
    void reset_count();

    bool update_mock_edges(int32_t delta_counts); // Explicit fixture input only
    int32_t get_raw_counts() const { return raw_counts_; }
    int32_t get_measured_um() const;

private:
    int32_t raw_counts_{0};
    int32_t scale_nm_per_count_{0}; // No nominal scale masquerading as calibration.
    bool counts_valid_{false};
};

} // namespace starz::firmware
