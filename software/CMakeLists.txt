# STARZ software tree.
#  machine/   machine software: host-testable core (motion, sequencing, safety
#             supervision, host link) + RP2350 board layer (separate Pico SDK
#             project in machine/rp2350) + serial CLI for a screenless machine.
#  firmware/  legacy non-hardware reference prototype, kept only for its tests
#             until the machine core supersedes them entirely.
# Nothing in this tree has been run on an energised machine (see docs/20).
option(STARZ_BUILD_MACHINE_HOST "Build the serial CLI for the machine host link" ON)
option(STARZ_BUILD_LEGACY_REFERENCE "Build the legacy reference prototype and its tests" ON)

add_subdirectory(machine)
if(STARZ_BUILD_LEGACY_REFERENCE)
  add_subdirectory(firmware)
endif()
if(STARZ_BUILD_HMI)
  add_subdirectory(hmi)
endif()
