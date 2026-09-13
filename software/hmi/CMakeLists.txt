find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(starz_hmi_reference_ui STATIC
  mainwindow.cpp
  serial_comm.cpp
  telemetry_view.cpp
)

target_include_directories(starz_hmi_reference_ui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
)

target_link_libraries(starz_hmi_reference_ui PUBLIC
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  Qt6::SerialPort
)

add_executable(starz_hmi main.cpp)
target_link_libraries(starz_hmi PRIVATE starz_hmi_reference_ui)

add_executable(test_reference_hmi tests/test_reference_hmi.cpp)
target_link_libraries(test_reference_hmi PRIVATE starz_hmi_reference_ui)
add_test(NAME test_reference_hmi COMMAND test_reference_hmi)
set_tests_properties(test_reference_hmi PROPERTIES
  TIMEOUT 30 ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
