cmake_minimum_required(VERSION 4.0)

add_library(
    global_common_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    global_common_library
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    global_common_library
    PUBLIC
        global_pch_library
        multi_index_lookup::multi_index_lookup
        base64_library
        magic_enum
)

if(WIN32)
    target_link_libraries(
        global_common_library
        PUBLIC
            psapi
    )
endif()

target_precompile_headers(
    global_common_library
    REUSE_FROM
        global_pch_library
)
