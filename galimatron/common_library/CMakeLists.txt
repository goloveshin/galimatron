cmake_minimum_required(VERSION 4.0)

add_library(
    galimatron_common_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    galimatron_common_library
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    galimatron_common_library
    PUBLIC
        global_common_library
)

target_precompile_headers(
    galimatron_common_library
    REUSE_FROM
        global_pch_library
)
