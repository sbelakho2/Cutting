#include "pio_step_dir.hpp"
#include <limits>
#include "../../shared/machine_spec.hpp"

namespace starz::firmware {

StepDirEngine::StepDirEngine(uint8_t /*step_pin*/, uint8_t /*dir_pin*/) {}

void StepDirEngine::init() {
    accumulated_steps_ = 0;
    forward_ = true;
    enabled_ = false;
    freq_hz_ = 0;
}

void StepDirEngine::set_direction(bool forward) {
    forward_ = forward;
}

void StepDirEngine::step_pulse() {
    if (!enabled_) return;
    if (accumulated_steps_ == std::numeric_limits<uint32_t>::max()) {
        enabled_ = false;
        freq_hz_ = 0;
        return;
    }
    accumulated_steps_++;
}

bool StepDirEngine::set_step_frequency_hz(uint32_t freq_hz) {
    if (freq_hz > spec::max_step_hz) {
        enabled_ = false;
        freq_hz_ = 0;
        return false;
    }
    freq_hz_ = freq_hz;
    return true;
}

void StepDirEngine::enable(bool en) {
    enabled_ = en;
    if (!en) freq_hz_ = 0;
}

} // namespace starz::firmware
