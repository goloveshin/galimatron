cmake_minimum_required(VERSION 4.0)

add_library(
    global_filesystem_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    global_filesystem_library
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    global_filesystem_library
    PUBLIC
        global_common_library
        global_pch_library
)

target_precompile_headers(
    global_filesystem_library
    REUSE_FROM
        global_pch_library
)
