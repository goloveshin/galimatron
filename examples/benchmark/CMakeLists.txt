cmake_minimum_required(VERSION 4.0)

# each aggregate is written into incoming_gal. version 0 holds the statement pack.
# the batch is written into incoming_gal/console.gal. it adds the later versions.
#
set(BENCHMARK_AGGREGATE_COUNT 500 CACHE STRING
    "Number of aggregates in the benchmark."
)
set(BENCHMARK_VERSION_COUNT 10 CACHE STRING
    "Number of versions in the benchmark, including version 0."
)
set(BENCHMARK_TAG_COUNT 2 CACHE STRING
    "Number of dynamic tags copied into each new version."
)

file(REMOVE_RECURSE
    ${CMAKE_CURRENT_SOURCE_DIR}/intermediate
    ${CMAKE_CURRENT_SOURCE_DIR}/actual_gal
)

set(BENCHMARK_INCOMING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/incoming_gal)
set(BENCHMARK_PROTOCOL_GAL ${BENCHMARK_INCOMING_DIR}/protocol.gal)
set(BENCHMARK_CONSOLE_GAL ${BENCHMARK_INCOMING_DIR}/console.gal)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/create_initial_protocol.cmake)

# not part of the normal build. each run starts with empty output directories
#
add_custom_target(
    benchmark
    COMMAND
        ${CMAKE_COMMAND} -E rm -rf
            ${CMAKE_CURRENT_SOURCE_DIR}/intermediate
            ${CMAKE_CURRENT_SOURCE_DIR}/actual_gal
    COMMAND
        $<TARGET_FILE:galimatron_executable>
            --gal-config-file ${CMAKE_CURRENT_SOURCE_DIR}/gal.cfg
            --log-config-file ${CMAKE_CURRENT_SOURCE_DIR}/log.cfg
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        galimatron_executable
    COMMENT
        "Benchmark: ${BENCHMARK_AGGREGATE_COUNT} aggregates, ${BENCHMARK_VERSION_COUNT} versions, ${BENCHMARK_TAG_COUNT} dynamic tags"
    VERBATIM
)

# not part of the normal build. 32 runs, then incoming_gal is restored to the cache counts' source
# by the script's last generate of 500 / 10 / 2
#
add_custom_target(
    benchmark_matrix
    COMMAND
        ${CMAKE_COMMAND}
            -DGALIMATRON_EXECUTABLE=$<TARGET_FILE:galimatron_executable>
            -DBENCHMARK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/benchmark_matrix.cmake
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        galimatron_executable
    COMMENT
        "Benchmark matrix: aggregates 10/20/50/100, versions 10/20/50/100, tags 2/8"
    USES_TERMINAL
    VERBATIM
)

# not part of the normal build. 12 runs of static payload tags, then the same restore of 500 / 10 / 2 dynamic
#
add_custom_target(
    benchmark_matrix_static
    COMMAND
        ${CMAKE_COMMAND}
            -DBENCHMARK_MATRIX_KIND=static
            -DGALIMATRON_EXECUTABLE=$<TARGET_FILE:galimatron_executable>
            -DBENCHMARK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/benchmark_matrix.cmake
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        galimatron_executable
    COMMENT
        "Benchmark matrix, static tags: aggregates 10/20/50/500, versions 10, tags 1/50/100"
    USES_TERMINAL
    VERBATIM
)

# same static grid, gal-static-no-xslt.cfg, so the language xslt and Saxon are absent
#
add_custom_target(
    benchmark_matrix_static_no_xslt
    COMMAND
        ${CMAKE_COMMAND}
            -DBENCHMARK_MATRIX_KIND=static-no-xslt
            -DGALIMATRON_EXECUTABLE=$<TARGET_FILE:galimatron_executable>
            -DBENCHMARK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/benchmark_matrix.cmake
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        galimatron_executable
    COMMENT
        "Benchmark matrix, static tags, no xslt: aggregates 10/20/50/500, versions 10, tags 1/50/100"
    USES_TERMINAL
    VERBATIM
)
