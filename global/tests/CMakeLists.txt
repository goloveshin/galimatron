cmake_minimum_required(VERSION 4.0)

add_executable(
    global_tests
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)
list(FILTER SOURCES EXCLUDE REGEX "global_test_data/")

target_sources(
    global_tests
    PRIVATE
        ${SOURCES}
)

target_include_directories(
    global_tests
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    global_tests
    PUBLIC
        global_common_library
        global_filesystem_library
        doctest::doctest
)

target_precompile_headers(
    global_tests
    REUSE_FROM
        global_pch_library
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config_tests.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/config_tests.hpp
    @ONLY
)

file(REMOVE_RECURSE
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/global_test_data
)
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
file(RENAME
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_data
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/global_test_data
)
