# inja's CMakeLists runs scripts/update_single_include.sh at configure time.
# That shell script does not run on Windows, so this writes a list that only exposes the headers.

if(NOT DEFINED SRC)
    message(FATAL_ERROR "SRC is required")
endif()

file(WRITE "${SRC}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 4.0)

add_library(inja INTERFACE)
add_library(pantor::inja ALIAS inja)
target_include_directories(
    inja
    SYSTEM
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(
    inja
    INTERFACE
        nlohmann_json::nlohmann_json
)
]=])
