# each executable's own exit code decides the step
if(NOT DEFINED GLOBAL_TESTS OR NOT DEFINED GALIMATRON_TESTS OR NOT DEFINED UUID_PROTOCOL_TESTS)
    message(FATAL_ERROR "run_tests.cmake needs GLOBAL_TESTS, GALIMATRON_TESTS, and UUID_PROTOCOL_TESTS")
endif()

set(test_exes
    "${GLOBAL_TESTS}"
    "${GALIMATRON_TESTS}"
    "${UUID_PROTOCOL_TESTS}"
)

# --success prints the suite, the case, and the open subcase on a passing run.
# --duration prints how long each test case took.
# GALIMATRON_ABORT_AFTER=1 stops after the first failed assertion. A local run leaves it unset.
set(doctest_args --success --duration)
if("$ENV{GALIMATRON_ABORT_AFTER}" STREQUAL "1")
    list(APPEND doctest_args --abort-after=1)
    message(STATUS "tests stop after the first failed assertion")
endif()

foreach(test_exe IN LISTS test_exes)
    message(STATUS "run ${test_exe}")
    execute_process(
        COMMAND "${test_exe}" ${doctest_args}
        WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
        RESULT_VARIABLE test_result
    )
    if(NOT test_result EQUAL 0)
        message(FATAL_ERROR "${test_exe} failed: ${test_result}")
    endif()
endforeach()
