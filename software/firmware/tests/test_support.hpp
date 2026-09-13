#pragma once
#include <stdexcept>
#include <string>

// Unlike assert(), every check and its expression execute under NDEBUG/Release.
inline void require_check(bool passed, const char* expression, const char* file, int line) {
    if (!passed) throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": " + expression);
}
#define CHECK(expression) require_check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)