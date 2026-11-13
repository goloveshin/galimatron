cmake_minimum_required(VERSION 4.0)

add_executable(
    galimatron_executable
)

set_target_properties(
    galimatron_executable
    PROPERTIES
        OUTPUT_NAME "galimatron"
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    galimatron_executable
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    galimatron_executable
    PUBLIC
        galimatron_runtime_library
        galimatron_executable_library
        Boost::program_options
)

target_precompile_headers(
    galimatron_executable
    REUSE_FROM
        global_pch_library
)
