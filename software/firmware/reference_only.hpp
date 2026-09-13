#pragma once

// Defense in depth, not a security boundary. There is no hardware implementation.
#if !defined(STARZ_REFERENCE_ONLY) || STARZ_REFERENCE_ONLY != 1
#error "Use the host-only STARZ reference CMake target. NOT FOR MACHINE OPERATION."
#endif
#if defined(PICO_ON_DEVICE) || defined(PICO_RP2040) || defined(PICO_RP2350) || defined(PICO_BUILD)
#error "This reference controller must not be built as production or Pico firmware."
#endif