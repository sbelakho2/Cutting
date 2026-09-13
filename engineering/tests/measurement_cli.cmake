if(NOT DEFINED ANALYSIS_EXE OR NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "Measurement executable/output directory required")
endif()
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
function(expect_code expected label)
  execute_process(COMMAND "${ANALYSIS_EXE}" ${ARGN}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(NOT result EQUAL expected)
    message(FATAL_ERROR "${label}: expected ${expected}, got ${result}\n${output}\n${error}")
  endif()
endfunction()
expect_code(0 "Synthetic report" --report "${OUTPUT_DIR}/synthetic")
expect_code(2 "Factory accuracy remains failed" --report "${OUTPUT_DIR}/strict" --require-factory-accuracy)
expect_code(0 "Fit then independently verify" --report "${OUTPUT_DIR}/fit"
  --fit-observations "${OUTPUT_DIR}/synthetic/synthetic-fit.csv"
  --verify-observations "${OUTPUT_DIR}/synthetic/synthetic-validation.csv")
expect_code(0 "Read back saved map" --report "${OUTPUT_DIR}/map"
  --calibration "${OUTPUT_DIR}/fit/input-derived-map.csv"
  --verify-observations "${OUTPUT_DIR}/synthetic/synthetic-validation.csv")
file(WRITE "${OUTPUT_DIR}/bad-validation.csv" "from_count,to_count,reference_mm,reference_uncertainty_mm\n0,4096,101.0,0.001\n")
expect_code(2 "Reject calibration residual" --report "${OUTPUT_DIR}/bad-residual"
  --calibration "${OUTPUT_DIR}/fit/input-derived-map.csv" --verify-observations "${OUTPUT_DIR}/bad-validation.csv")
file(READ "${OUTPUT_DIR}/bad-residual/status.json" bad_status)
if(NOT bad_status MATCHES "\"validation_residual_failures\":1" OR NOT bad_status MATCHES "\"physical_calibration_qualified\":false")
  message(FATAL_ERROR "Calibration failure status is not fail closed")
endif()
file(WRITE "${OUTPUT_DIR}/incomplete.csv" "count,travel_mm\n0,0\n4095,100\n")
expect_code(1 "Reject malformed map" --report "${OUTPUT_DIR}/invalid-map"
  --calibration "${OUTPUT_DIR}/incomplete.csv" --verify-observations "${OUTPUT_DIR}/bad-validation.csv")
expect_code(1 "Reject missing validation" --report "${OUTPUT_DIR}/missing-verification"
  --calibration "${OUTPUT_DIR}/fit/input-derived-map.csv")
expect_code(1 "Reject unknown option" --report "${OUTPUT_DIR}/unknown" --enable-hardware)
expect_code(1 "Reject identical fit/verify input" --report "${OUTPUT_DIR}/reused-data"
  --fit-observations "${OUTPUT_DIR}/synthetic/synthetic-fit.csv"
  --verify-observations "${OUTPUT_DIR}/synthetic/synthetic-fit.csv")
expect_code(1 "Protect original input file" --report "${OUTPUT_DIR}/fit"
  --calibration "${OUTPUT_DIR}/fit/input-derived-map.csv"
  --verify-observations "${OUTPUT_DIR}/synthetic/synthetic-validation.csv")
file(REMOVE "${OUTPUT_DIR}/fit-alias.csv" "${OUTPUT_DIR}/map-alias.csv")
file(CREATE_LINK "${OUTPUT_DIR}/synthetic/synthetic-fit.csv" "${OUTPUT_DIR}/fit-alias.csv" RESULT link_result)
if(NOT link_result STREQUAL "0")
  message(FATAL_ERROR "Cannot create hard-link regression fixture: ${link_result}")
endif()
expect_code(1 "Reject hard-linked fit/verify input" --report "${OUTPUT_DIR}/reused-hardlink"
  --fit-observations "${OUTPUT_DIR}/synthetic/synthetic-fit.csv" --verify-observations "${OUTPUT_DIR}/fit-alias.csv")
file(CREATE_LINK "${OUTPUT_DIR}/fit/input-derived-map.csv" "${OUTPUT_DIR}/map-alias.csv" RESULT link_result)
if(NOT link_result STREQUAL "0")
  message(FATAL_ERROR "Cannot create output hard-link fixture: ${link_result}")
endif()
expect_code(1 "Protect hard-linked input from output" --report "${OUTPUT_DIR}/fit"
  --calibration "${OUTPUT_DIR}/map-alias.csv" --verify-observations "${OUTPUT_DIR}/synthetic/synthetic-validation.csv")
foreach(name invalid-map missing-verification unknown reused-data reused-hardlink)
  if(EXISTS "${OUTPUT_DIR}/${name}/status.json")
    message(FATAL_ERROR "Invalid arguments initialized output status for ${name}")
  endif()
endforeach()