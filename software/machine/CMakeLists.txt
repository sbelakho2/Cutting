# STARZ machine software (host-buildable core). The RP2350 target build lives in
# software/machine/rp2350 and is a separate CMake project (Pico SDK).
add_library(starz_machine_core STATIC
  core/profile.cpp
  core/controller.cpp
  core/protocol.cpp)
target_include_directories(starz_machine_core PUBLIC core ${CMAKE_SOURCE_DIR}/shared)
target_compile_features(starz_machine_core PUBLIC cxx_std_20)

add_subdirectory(tests)
if(STARZ_BUILD_MACHINE_HOST)
  add_subdirectory(host)
endif()
