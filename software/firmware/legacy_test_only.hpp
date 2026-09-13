#pragma once
#include "reference_only.hpp"
#if !defined(STARZ_ENABLE_LEGACY_TEST_DOUBLES) || STARZ_ENABLE_LEGACY_TEST_DOUBLES != 1
#error "Legacy counter doubles are quarantined. They are NOT PIO or hardware drivers."
#endif