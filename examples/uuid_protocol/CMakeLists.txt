cmake_minimum_required(VERSION 4.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions.cmake)

# shared by every history step. each step is a numbered folder under steps/.

add_subdirectory(common)
add_subdirectory(steps)
add_subdirectory(tests)
