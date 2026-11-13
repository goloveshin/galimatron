cmake_minimum_required(VERSION 4.0)

add_executable(
    uuid_protocol_tests
    main.cpp
)

target_link_libraries(
    uuid_protocol_tests
    PRIVATE
        global_common_library
        doctest::doctest
        Boost::process
)

target_precompile_headers(
    uuid_protocol_tests
    REUSE_FROM
        global_pch_library
)

# uuid_protocol_add_step records each client and server
get_property(uuid_protocol_endpoints GLOBAL PROPERTY UUID_PROTOCOL_ENDPOINTS)
if(NOT uuid_protocol_endpoints)
    message(FATAL_ERROR "uuid protocol steps were not found")
endif()

add_dependencies(
    uuid_protocol_tests
    ${uuid_protocol_endpoints}
)
