add_library(starz_firmware_core STATIC
  motion_planner.cpp
  safety_controller.cpp
  firmware_controller.cpp
)

target_compile_definitions(starz_firmware_core PUBLIC STARZ_REFERENCE_ONLY=1)
target_compile_features(starz_firmware_core PUBLIC cxx_std_20)
add_library(starz_reference_controls ALIAS starz_firmware_core)

target_include_directories(starz_firmware_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
)

add_executable(test_firmware_motion tests/test_firmware_motion.cpp)
target_link_libraries(test_firmware_motion PRIVATE starz_firmware_core)
add_test(NAME test_firmware_motion COMMAND test_firmware_motion)
set_tests_properties(test_firmware_motion PROPERTIES TIMEOUT 30)

add_executable(test_reference_protocol tests/test_protocol.cpp)
target_link_libraries(test_reference_protocol PRIVATE starz_firmware_core)
add_test(NAME test_reference_protocol COMMAND test_reference_protocol)
set_tests_properties(test_reference_protocol PROPERTIES TIMEOUT 30)

option(STARZ_BUILD_LEGACY_TEST_DOUBLES "Build quarantined, non-PIO counter test doubles" OFF)
if(STARZ_BUILD_LEGACY_TEST_DOUBLES)
  add_library(starz_legacy_test_doubles STATIC pio_step_dir.cpp pio_quadrature.cpp)
  target_compile_features(starz_legacy_test_doubles PUBLIC cxx_std_20)
  target_compile_definitions(starz_legacy_test_doubles PUBLIC
    STARZ_REFERENCE_ONLY=1 STARZ_ENABLE_LEGACY_TEST_DOUBLES=1)
endif()
