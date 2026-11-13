cmake_minimum_required(VERSION 4.0)

add_library(
    uuid_protocol_common_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    uuid_protocol_common_library
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    uuid_protocol_common_library
    PUBLIC
        global_common_library
)

target_precompile_headers(
    uuid_protocol_common_library
    REUSE_FROM
        global_pch_library
)
