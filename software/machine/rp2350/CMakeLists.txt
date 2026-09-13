cmake_minimum_required(VERSION 3.24)

# This is deliberately separate from the desktop/host project: it must be
# configured with the Pico SDK and produces the production target image plus a
# distinct IF-B01 factory-fixture image.
if(NOT DEFINED PICO_SDK_PATH)
  message(FATAL_ERROR "Set PICO_SDK_PATH to the installed Pico SDK (for example: $HOME/pico-sdk).")
endif()
if(NOT DEFINED PICO_BOARD)
  set(PICO_BOARD pico2 CACHE STRING "Target board" FORCE)
endif()
# Pico 2 SC1631 has 4 MiB. The linker receives 8 KiB less, permanently
# reserving the final two 4 KiB sectors for the A/B calibration journal.
set(PICO_FLASH_SIZE_BYTES "(4 * 1024 * 1024 - 2 * 4096)" CACHE STRING "Application flash excluding settings journal" FORCE)
# Firmware never launches core 1. This lets pico_flash safely stop only core 0
# interrupts while writing the journal, without an unavailable core-1 lockout.
set(PICO_FLASH_ASSUME_CORE1_SAFE 1 CACHE BOOL "Core 1 is not used by STARZ firmware" FORCE)
set(PICO_FLASH_ASSERT_ON_UNSAFE 0 CACHE BOOL "Return flash-safe errors instead of asserting" FORCE)
set(PICO_NO_PICOTOOL 0 CACHE BOOL "Use the installed picotool to generate a UF2 install image" FORCE)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(starz_machine_rp2350 LANGUAGES C CXX ASM)

pico_sdk_init()

add_executable(starz-machine
  main.cpp
  board_rp2350.cpp
  ../core/profile.cpp
  ../core/controller.cpp
  ../core/protocol.cpp)

target_compile_features(starz-machine PRIVATE cxx_std_20)
target_compile_options(starz-machine PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  $<$<COMPILE_LANGUAGE:CXX>:-Wconversion>
  $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
target_include_directories(starz-machine PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../core
  ${CMAKE_CURRENT_LIST_DIR}/../../../shared)
target_link_libraries(starz-machine PRIVATE
  pico_stdlib
  pico_flash
  hardware_clocks
  hardware_flash
  hardware_gpio
  hardware_pio
  hardware_watchdog)

pico_generate_pio_header(starz-machine ${CMAKE_CURRENT_LIST_DIR}/step_gen.pio)
pico_generate_pio_header(starz-machine ${CMAKE_CURRENT_LIST_DIR}/quadrature.pio)
pico_enable_stdio_usb(starz-machine 1)
pico_enable_stdio_uart(starz-machine 0)
pico_add_uf2_output(starz-machine)
pico_add_extra_outputs(starz-machine)

# Dedicated bare-board diagnostic image. It deliberately has no Controller or
# motion step PIO. Build/flash it only on the IF-B01 factory fixture, never on
# an assembled production machine.
add_executable(if-b01-factory-test
  factory_test.cpp)

target_compile_features(if-b01-factory-test PRIVATE cxx_std_20)
target_compile_options(if-b01-factory-test PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wall>
  $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
  $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
  $<$<COMPILE_LANGUAGE:CXX>:-Wconversion>
  $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
target_include_directories(if-b01-factory-test PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../../../shared)
target_link_libraries(if-b01-factory-test PRIVATE
  pico_stdlib
  hardware_gpio
  hardware_pio
  hardware_watchdog)

pico_generate_pio_header(if-b01-factory-test ${CMAKE_CURRENT_LIST_DIR}/quadrature.pio
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/if-b01-factory-test-pio)
pico_enable_stdio_usb(if-b01-factory-test 1)
pico_enable_stdio_uart(if-b01-factory-test 0)
pico_add_uf2_output(if-b01-factory-test)
pico_add_extra_outputs(if-b01-factory-test)
