cmake_minimum_required(VERSION 4.0)

# dynamic CRT: /MD in Release, /MDd in Debug (VCRUNTIME*.dll)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

project(GalimatronProject VERSION 1.0.0 LANGUAGES CXX C)

# SaxonC embeds a GraalVM native image. The Windows default reserve is 1MB.
# Release fills that stack during an XSLT, and the VM cannot enable its yellow zone.
# 8MB is reserved. Pages are committed as the stack grows.
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8388608")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(CXX_STANDARD=${CMAKE_CXX_STANDARD})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

# Linux x86-64 packages are built for x86-64-v2, the level a VirtualBox guest exposes.
# The flag is set before the dependencies so their static libraries carry the same ISA note.
if(UNIX AND NOT APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
    add_compile_options(-march=x86-64-v2)
endif()

# the include is needed so that it is visible to everyone
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies.cmake")

# we mostly use an explicit .hpp path relative to this directory,
# except where that is not possible or not needed
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Externals dependencies

add_subdirectory(externals)

# Global library

add_subdirectory(global)

# Galimatron project

add_subdirectory(galimatron)

# Examples of using Galimatron

add_subdirectory(examples)

# generates the uuid protocol sources, builds every test executable, then runs them
# one cmake process, so a crashing helper inside a suite does not become this step's exit code
add_custom_target(
    run_tests
    COMMAND ${CMAKE_COMMAND}
        -DGLOBAL_TESTS=$<TARGET_FILE:global_tests>
        -DGALIMATRON_TESTS=$<TARGET_FILE:galimatron_tests>
        -DUUID_PROTOCOL_TESTS=$<TARGET_FILE:uuid_protocol_tests>
        -DTEST_WORKING_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_tests.cmake
    DEPENDS
        global_tests
        galimatron_tests
        uuid_protocol_tests
    USES_TERMINAL
    VERBATIM
)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/cpack.cmake")
