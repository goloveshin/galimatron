cmake_minimum_required(VERSION 4.0)

add_library(
    base64_library
    INTERFACE
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp)

target_sources(
    base64_library
    INTERFACE
        ${SOURCES}
)

target_include_directories(
    base64_library
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
