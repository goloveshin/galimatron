cmake_minimum_required(VERSION 4.0)

add_library(
    galimatron_executable_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    galimatron_executable_library
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    galimatron_executable_library
    PUBLIC
        galimatron_common_library
    PRIVATE
        Boost::program_options
)

target_precompile_headers(
    galimatron_executable_library
    REUSE_FROM
        global_pch_library
)
