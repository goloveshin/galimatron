cmake_minimum_required(VERSION 4.0)

add_executable(
    global_tests
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    global_tests
    PRIVATE
        ${SOURCES}
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
