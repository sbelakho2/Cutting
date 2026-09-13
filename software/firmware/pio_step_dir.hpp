#pragma once
#include <cstdint>
#include "legacy_test_only.hpp"

namespace starz::firmware {

// Legacy in-memory counter test double. NO pulses, GPIO, timing or PIO exist.
class StepDirEngine {
public:
    StepDirEngine(uint8_t step_pin, uint8_t dir_pin);

    void init();
    void set_direction(bool forward);
    void step_pulse();
    bool set_step_frequency_hz(uint32_t freq_hz);
    void enable(bool en);

    uint32_t get_accumulated_steps() const { return accumulated_steps_; }
    void reset_steps() { accumulated_steps_ = 0; }

private:
    bool forward_{true};
    bool enabled_{false};
    uint32_t freq_hz_{0};
    uint32_t accumulated_steps_{0};
};

} // namespace starz::firmware
