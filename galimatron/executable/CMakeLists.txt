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

# --version and the Windows file version read project() VERSION from the root project
set(GALIMATRON_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.hpp.in
    ${GALIMATRON_GENERATED_DIR}/galimatron/version.hpp
    @ONLY
)
target_include_directories(
    galimatron_executable
    PRIVATE
        ${GALIMATRON_GENERATED_DIR}
)
if(WIN32)
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/galimatron.rc.in
        ${GALIMATRON_GENERATED_DIR}/galimatron.rc
        @ONLY
    )
    target_sources(
        galimatron_executable
        PRIVATE
            ${GALIMATRON_GENERATED_DIR}/galimatron.rc
    )
endif()
