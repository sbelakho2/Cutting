if(NOT DEFINED ANALYSIS_EXE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "Missing shaft-analysis test paths")
endif()
set(cases "0:0.1" "2:3" "2:nan" "2:0.01" "2:0.3,2:0.2" "2:0.3,1:0.4" "2:0.3," "2:0.3junk")
foreach(levels IN LISTS cases)
  execute_process(COMMAND "${ANALYSIS_EXE}" --report "${OUTPUT_DIR}" --meshes "${levels}"
    RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET TIMEOUT 10)
  if(NOT result STREQUAL "1")
    message(FATAL_ERROR "Invalid mesh request '${levels}' was not rejected with exit1: ${result}")
  endif()
endforeach()
foreach(request IN ITEMS
  "--mesh-optimizer"
  "--mesh-optimizer;unknown"
  "--mesh-optimizer;HighOrder"
    "--focus-region;unknown"
    "--focus-region;whole_nominal_shaft"
    "--focus-region;blind_bore_and_cosmetic_thread"
    "--background-root;0.2"
    "--focus-growth;0.7"
    "--focus-region;A12_groove;--focus-growth;0.01"
    "--focus-region;A12_groove;--focus-growth;1.01"
    "--focus-region;A12_groove;--background-root;nan"
    "--focus-region;A12_groove;--background-root;0.01;--meshes;2:0.02"
    "--focus-region;A12_groove;--background-root;3;--meshes;2:0.02"
    "--focus-region;A12_groove;--meshes;2:0.004"
    "--focus-feature"
    "--focus-feature;unknown"
    "--focus-feature;A12_S18_5;--meshes;2:0.02"
    "--focus-region;M15_relief;--focus-feature;A12_S18_5;--meshes;2:0.02"
    "--focus-region;A12_groove;--focus-feature;M15_slot_exit;--meshes;2:0.02"
    "--focus-region;A12_groove;--focus-feature;pulley_key_floor;--meshes;2:0.02"
    "--focus-feature;concave_fillet_tori;--focus-region;M15_relief;--meshes;2:0.02"
    "--focus-feature;concave_fillet_tori;--background-root;0.2"
    "--focus-region")
  execute_process(COMMAND "${ANALYSIS_EXE}" --report "${OUTPUT_DIR}" ${request}
    RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET TIMEOUT 10)
  if(NOT result STREQUAL "1")
    message(FATAL_ERROR "Invalid focused request '${request}' was not rejected: ${result}")
  endif()
endforeach()
foreach(phases IN ITEMS 0 7 721 12.5 nan)
  execute_process(COMMAND "${ANALYSIS_EXE}" --report "${OUTPUT_DIR}" --phases "${phases}"
    RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET TIMEOUT 10)
  if(NOT result STREQUAL "1")
    message(FATAL_ERROR "Invalid phase count '${phases}' was not rejected: ${result}")
  endif()
endforeach()