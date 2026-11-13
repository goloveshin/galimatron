# one endpoint source for every step.
# port "default" listens on client_and_server_t::port. the arguments after runtime_dir are the libraries
function(uuid_protocol_add_endpoint target log_file is_client port runtime_dir)
    add_executable(
        ${target}
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../executable/endpoint_main.cpp
    )

    target_compile_definitions(
        ${target}
        PRIVATE
            UUID_PROTOCOL_LOG_FILE="${log_file}"
            UUID_PROTOCOL_IS_CLIENT=${is_client}
    )
    if(NOT port STREQUAL "default")
        target_compile_definitions(
            ${target}
            PRIVATE
                UUID_PROTOCOL_PORT=${port}
        )
    endif()

    target_link_libraries(
        ${target}
        PRIVATE
            ${ARGN}
    )

    target_precompile_headers(
        ${target}
        REUSE_FROM
            global_pch_library
    )

    set_target_properties(
        ${target}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${runtime_dir}"
    )
endfunction()

# stamp written under the step's binary dir, in generated/.
function(uuid_protocol_stamp_file binary_dir step out_var)
    set(stamp "${binary_dir}/uuid_protocol_${step}_generating_sources.stamp")
    cmake_path(NORMAL_PATH stamp)
    set(${out_var} "${stamp}" PARENT_SCOPE)
endfunction()

# one generator for every step that has generated/gal.cfg.
# the stamp is the only output. a conversion file galimatron keeps is still compiled:
# by_hand globs generated_conversion, and that glob is not an output of this command.
# CONFIGURE_DEPENDS reruns cmake on the next build when a globbed file appears or disappears.
# the caller sees UUID_PROTOCOL_<step>_GENERATING_STAMP.
function(uuid_protocol_add_generator step generated_dir)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
    uuid_protocol_stamp_file("${CMAKE_CURRENT_BINARY_DIR}/generated" "${step}" stamp)

    file(GLOB_RECURSE gal CONFIGURE_DEPENDS "${generated_dir}/incoming_gal/*.gal")
    file(GLOB incoming_conversion_state CONFIGURE_DEPENDS "${generated_dir}/incoming_gal/*.xml")
    file(GLOB conversion_state CONFIGURE_DEPENDS "${generated_dir}/actual_gal/*.xml")
    file(GLOB_RECURSE xslt CONFIGURE_DEPENDS "${generated_dir}/xslt/*")
    if(NOT gal OR NOT xslt)
        message(FATAL_ERROR "uuid protocol step ${step} generator inputs were not found")
    endif()

    add_custom_command(
        OUTPUT
            ${stamp}
        COMMAND
            $<TARGET_FILE:galimatron_executable>
                --log-config-file ${generated_dir}/log.cfg
                --gal-config-file ${generated_dir}/gal.cfg
        COMMAND
            ${CMAKE_COMMAND} -E touch ${stamp}
        DEPENDS
            galimatron_executable
            ${generated_dir}/log.cfg
            ${generated_dir}/gal.cfg
            ${gal}
            ${incoming_conversion_state}
            ${conversion_state}
            ${xslt}
        WORKING_DIRECTORY
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT
            "Generating code for Uuid Protocol step ${step}"
        VERBATIM
    )

    add_custom_target(
        uuid_protocol_${step}_generating_sources
        DEPENDS
            ${stamp}
    )

    set(UUID_PROTOCOL_${step}_GENERATING_STAMP "${stamp}" PARENT_SCOPE)
endfunction()

# the static library of generated_dto sources. conversion .cpp stays in the by_hand library.
# a dto file written during this build is compiled on the next build.
function(uuid_protocol_add_generated_library step generated_dir stamp)
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        "${generated_dir}/generated_dto/*.hpp"
        "${generated_dir}/generated_dto/*.cpp"
    )
    set(cpp_sources)
    foreach(source ${sources})
        if(source MATCHES "\\.cpp$")
            list(APPEND cpp_sources "${source}")
        endif()
    endforeach()
    if(NOT cpp_sources)
        set(UUID_PROTOCOL_${step}_HAS_GENERATED_LIBRARY FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(
        uuid_protocol_${step}_generated_library
        STATIC
        ${sources}
    )

    target_include_directories(
        uuid_protocol_${step}_generated_library
        PUBLIC
            ${generated_dir}/generated_dto
    )

    target_link_libraries(
        uuid_protocol_${step}_generated_library
        PUBLIC
            uuid_protocol_common_library
            alpaca
    )

    target_precompile_headers(
        uuid_protocol_${step}_generated_library
        REUSE_FROM
            global_pch_library
    )

    set_source_files_properties(
        ${cpp_sources}
        PROPERTIES
            OBJECT_DEPENDS "${stamp}"
    )
    add_dependencies(
        uuid_protocol_${step}_generated_library
        uuid_protocol_${step}_generating_sources
    )

    set(UUID_PROTOCOL_${step}_HAS_GENERATED_LIBRARY TRUE PARENT_SCOPE)
endfunction()

# by_hand library, and the client and server, for the step whose CMakeLists calls this.
# port "default" listens on client_and_server_t::port.
# generated/gal.cfg adds the generator. generated_dto sources are globbed into their library.
# generated_conversion/*.cpp is globbed into this by_hand library, including a file the generator keeps.
# a step with by_hand/conversion defines those apply functions itself, so the generated .cpp stays unlinked.
# a step without by_hand/server_state.cpp compiles the io_message-only server state.
# that file, and a step's own server_state.cpp, include common/io_message.hpp
# so io_message and server_io_message are one object file.
# executables land in bin/uuid_protocol/steps/<folder>/.
# each call appends the client and server to the global property UUID_PROTOCOL_ENDPOINTS.
function(uuid_protocol_add_step step port)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME folder)

    set(by_hand uuid_protocol_${step}_by_hand_library)
    set(generated_library uuid_protocol_${step}_generated_library)
    set(generated_dir "${CMAKE_CURRENT_SOURCE_DIR}/generated")
    set(runtime_dir "${CMAKE_BINARY_DIR}/bin/uuid_protocol/steps/${folder}")
    set(log_dir "uuid_protocol/steps/${folder}")

    set(has_generator FALSE)
    set(has_generated FALSE)
    if(EXISTS "${generated_dir}/gal.cfg")
        uuid_protocol_add_generator(${step} ${generated_dir})
        set(has_generator TRUE)
        uuid_protocol_add_generated_library(
            ${step}
            ${generated_dir}
            "${UUID_PROTOCOL_${step}_GENERATING_STAMP}"
        )
        if(UUID_PROTOCOL_${step}_HAS_GENERATED_LIBRARY)
            set(has_generated TRUE)
        endif()
    endif()

    add_library(${by_hand} STATIC)

    file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/by_hand/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/by_hand/*.cpp
    )
    target_sources(
        ${by_hand}
        PRIVATE
            ${sources}
    )

    # the generated headers declare handle and log. this library defines them.
    if(has_generator)
        target_include_directories(
            ${by_hand}
            PUBLIC
                ${generated_dir}/generated_dto
        )
    endif()

    set(conversion_dir "${generated_dir}/generated_conversion")
    if(EXISTS "${conversion_dir}")
        target_include_directories(
            ${by_hand}
            PUBLIC
                ${conversion_dir}
        )
    endif()

    # apply is one function per source type. copies kept from earlier steps declare the same function.
    set(conversion_cpp "")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/by_hand/conversion")
        file(GLOB conversion_cpp CONFIGURE_DEPENDS "${conversion_dir}/*.cpp")
    endif()
    if(conversion_cpp)
        target_sources(
            ${by_hand}
            PRIVATE
                ${conversion_cpp}
        )
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/by_hand/server_state.cpp")
        target_sources(
            ${by_hand}
            PRIVATE
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/server_state_default.cpp"
        )
    endif()

    target_link_libraries(
        ${by_hand}
        PUBLIC
            uuid_protocol_common_library
    )

    target_precompile_headers(
        ${by_hand}
        REUSE_FROM
            global_pch_library
    )

    if(has_generator)
        file(GLOB_RECURSE by_hand_cpp CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/by_hand/*.cpp)
        set(wait_sources ${by_hand_cpp})
        if(conversion_cpp)
            list(APPEND wait_sources ${conversion_cpp})
        endif()
        set_source_files_properties(
            ${wait_sources}
            PROPERTIES
                OBJECT_DEPENDS "${UUID_PROTOCOL_${step}_GENERATING_STAMP}"
        )
        add_dependencies(
            ${by_hand}
            uuid_protocol_${step}_generating_sources
        )
    endif()

    if(has_generated)
        set(libs ${by_hand} ${generated_library})
    else()
        set(libs ${by_hand})
    endif()

    uuid_protocol_add_endpoint(
        uuid_protocol_${step}_client
        "${log_dir}/uuid_protocol_${step}_client.log"
        1
        "${port}"
        "${runtime_dir}"
        ${libs}
    )
    uuid_protocol_add_endpoint(
        uuid_protocol_${step}_server
        "${log_dir}/uuid_protocol_${step}_server.log"
        0
        "${port}"
        "${runtime_dir}"
        ${libs}
    )

    set_property(
        GLOBAL APPEND PROPERTY
        UUID_PROTOCOL_ENDPOINTS
            uuid_protocol_${step}_client
            uuid_protocol_${step}_server
    )
endfunction()
