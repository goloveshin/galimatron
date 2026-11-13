cmake_minimum_required(VERSION 4.0)

add_library(
    galimatron_runtime_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    galimatron_runtime_library
    PRIVATE
        ${SOURCES}
)

target_link_libraries(
    galimatron_runtime_library
    PUBLIC
        galimatron_translator_library
        leaf_spy::leaf_spy
        global_filesystem_library
)

target_link_libraries(
    galimatron_runtime_library
    PRIVATE
        Boost::units
)

# std::stacktrace lives in libstdc++exp with GCC and Clang.
# MSVC provides it in the standard library.
if(NOT MSVC)
    target_link_libraries(
        galimatron_runtime_library
        PUBLIC
            stdc++exp
    )
endif()

target_precompile_headers(
    galimatron_runtime_library
    REUSE_FROM
        global_pch_library
)
