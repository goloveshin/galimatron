cmake_minimum_required(VERSION 4.0)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# uuid protocol. each numbered step is the next version.

add_subdirectory(uuid_protocol)

# wizard batch benchmark.
# counts are BENCHMARK_AGGREGATE_COUNT, BENCHMARK_VERSION_COUNT, and BENCHMARK_TAG_COUNT.
# not part of the normal build
add_subdirectory(benchmark)
