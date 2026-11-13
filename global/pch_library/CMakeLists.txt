cmake_minimum_required(VERSION 4.0)

add_library(
    global_pch_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)

target_sources(
    global_pch_library
    PRIVATE
        ${SOURCES}
)

target_compile_definitions(
    global_pch_library
    PUBLIC
        LOGME_INRELEASE
)

target_link_libraries(
    global_pch_library
    PUBLIC
        Boost::asio
        Boost::algorithm
        Boost::graph
        Boost::icl
        Boost::leaf
        Boost::program_options
        Boost::uuid
        Boost::multi_index
        logme
        base64_library
        magic_enum
        nlohmann_json
        pugixml
        range-v3
)

# Logme's control server calls Winsock. Asio used to pull ws2_32 in through the precompiled header.
if(WIN32)
    target_link_libraries(
        global_pch_library
        PUBLIC
            ws2_32
    )
endif()

target_precompile_headers(
    global_pch_library
    PRIVATE
        pch.hpp
)
