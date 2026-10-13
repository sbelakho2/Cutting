add_executable(test_machine test_machine.cpp)
target_link_libraries(test_machine PRIVATE starz_machine_core)
target_include_directories(test_machine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Source tree of the machine software: the F05 host check scans the PIO source
# and the two encoder-restart implementations for any blocking TX dependency.
target_compile_definitions(test_machine PRIVATE
  STARZ_MACHINE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/..")
foreach(t profile validate homing cycle correction slip stop safety board jog storage journal protocol
          f05_quadrature_init f07_strip_geometry f08_strip_closure f09_length_budget f11_part_clear f12_approval
          f33_jog_overflow f33_polarity)
  add_test(NAME machine.${t} COMMAND test_machine ${t})
endforeach()
