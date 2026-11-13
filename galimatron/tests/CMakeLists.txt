cmake_minimum_required(VERSION 4.0)

add_executable(
    galimatron_tests
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp)
list(FILTER SOURCES EXCLUDE REGEX "crash_child_main\\.cpp$")

target_sources(
    galimatron_tests
    PRIVATE
        ${SOURCES}
)

target_include_directories(
    galimatron_tests
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    galimatron_tests
    PUBLIC
        galimatron_runtime_library
        galimatron_executable_library
        doctest::doctest
        Boost::process
)

target_precompile_headers(
    galimatron_tests
    REUSE_FROM
        global_pch_library
)

add_executable(
    galimatron_crash_child
    runtime_library/crash_child_main.cpp
)

target_link_libraries(
    galimatron_crash_child
    PRIVATE
        galimatron_runtime_library
        global_common_library
)

target_precompile_headers(
    galimatron_crash_child
    REUSE_FROM
        global_pch_library
)

add_dependencies(
    galimatron_tests
    galimatron_crash_child
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config_tests.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/config_tests.hpp
    @ONLY
)

message(STATUS "remove ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/galimatron_test_data")
file(REMOVE_RECURSE
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/galimatron_test_data
)
message(STATUS "copy ${CMAKE_CURRENT_SOURCE_DIR}/test_data to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
message(STATUS "rename ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_data to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/galimatron_test_data")
file(RENAME
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_data
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/galimatron_test_data
)

# shared integration-test configs live in one place and are copied out to the cases
set(galimatron_test_data_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/galimatron_test_data)
set(galimatron_common_cfg ${CMAKE_CURRENT_SOURCE_DIR}/test_data/translator_library/common)
set(galimatron_default_cfg ${galimatron_common_cfg}/default)

set(galimatron_cases_with_common_gal
    translator_library/wizard_create_root_aggregate
    translator_library/wizard_batch/create
    translator_library/wizard_batch/hash
    translator_library/wizard_batch/hash_last
    translator_library/wizard_version_statements/batch
    translator_library/wizard_version_statements/in_version
    translator_library/wizard_create_aggregate/in_empty_chunk
    translator_library/wizard_create_aggregate/in_root_chunk
    translator_library/wizard_create_aggregate/list
    translator_library/wizard_create_aggregate/in_directory
    translator_library/wizard_copy_aggregate/one
    translator_library/wizard_continue_aggregate/one
    translator_library/wizard_delete_aggregate/list
    translator_library/wizard_delete_aggregate/open
    translator_library/wizard_delete_aggregate/physical
    translator_library/wizard_delete_aggregate/physically
    translator_library/wizard_delete_aggregate/all
    translator_library/wizard_delete_version/open
    translator_library/wizard_delete_version/list
    translator_library/wizard_collapse_version/range
    translator_library/wizard_collapse_version/with_anonymous
    translator_library/wizard_collapse_version/closed
    translator_library/wizard_collapse_version/reassign
    translator_library/wizard_collapse_version/stay_deleted
    translator_library/gal_version/version_1
    translator_library/gal_version/version_2
    translator_library/wizard_create_hash/root_only
    translator_library/wizard_create_hash/force
    translator_library/wizard_create_hash/aggregates
    translator_library/wizard_create_hash/last
    translator_library/wizard_delete_hash/versions
    translator_library/wizard_delete_hash/force
    translator_library/wizard_delete_hash/last
    translator_library/wizard_create_version/root_only
    translator_library/wizard_create_version/aggregates
    translator_library/wizard_create_version/deleted_tag
    translator_library/wizard_create_version/local_stays
    translator_library/wizard_create_version/forced_once
    translator_library/wizard_create_version/forced_local
    translator_library/wizard_create_version/own_stays
    translator_library/wizard_create_version/forced_own
    translator_library/cpp2gal/language
    translator_library/cpp2gal/formatting
    translator_library/cpp2gal/default_layout
    translator_library/cpp2gal/template_engine
    translator_library/cpp2gal/user_comments
    translator_library/cpp2gal/raw_string
    translator_library/validate/root_only_zero
    translator_library/validate/declarations_012
    translator_library/validate/declarations_empty
    translator_library/validate/aggregate_match
    translator_library/validate/deleted_suffix
    translator_library/validate/root_none
    translator_library/validate/root_two
    translator_library/validate/declarations_starts_with_1
    translator_library/validate/declarations_gap
    translator_library/validate/declarations_out_of_order
    translator_library/validate/definitions_missing
    translator_library/validate/definitions_extra
    translator_library/validate/definitions_order
    translator_library/validate/aggregate_shorter
    translator_library/validate/deleted_then_live
    translator_library/validate/deleted_gap
    translator_library/validate/deleted_resumed
    translator_library/validate/definitions_of_deleted
    translator_library/validate/definitions_missing_live
    translator_library/validate/wizard_not_chunk
    translator_library/validate/wizard_two_commands
    translator_library/validate/wizard_create_root_aggregate_command_file
    translator_library/validate/wizard_create_aggregate_command_file
    translator_library/validate/wizard_root_exists
    translator_library/validate/wizard_create_version_not_declarations
    translator_library/validate/wizard_batch_not_chunk
    translator_library/validate/wizard_batch_and_command
    translator_library/validate/wizard_batch_delete_hash
    translator_library/validate/wizard_create_hash_not_declarations
    translator_library/validate/wizard_delete_hash_not_declarations
    translator_library/validate/wizard_create_hash_version_missing
    translator_library/validate/wizard_delete_hash_version_missing
    translator_library/validate/wizard_create_hash_last_missing
    translator_library/validate/wizard_delete_hash_last_missing
    translator_library/validate/wizard_delete_aggregate_not_chunk
    translator_library/validate/wizard_delete_aggregate_missing
    translator_library/validate/wizard_delete_aggregate_root
    translator_library/validate/wizard_delete_aggregate_names_repeated
    translator_library/validate/wizard_delete_aggregate_already_deleted
    translator_library/validate/wizard_delete_aggregate_closed
    translator_library/validate/wizard_delete_aggregate_physically_closed
    translator_library/validate/wizard_delete_aggregate_physically_ordered
    translator_library/validate/wizard_copy_aggregate_missing
    translator_library/validate/wizard_copy_aggregate_closed
    translator_library/validate/wizard_copy_aggregate_present
    translator_library/validate/wizard_continue_aggregate_missing
    translator_library/validate/wizard_continue_aggregate_deleted
    translator_library/validate/wizard_continue_aggregate_present
    translator_library/validate/wizard_delete_version_closed
    translator_library/validate/wizard_delete_version_closed_listed
    translator_library/validate/wizard_delete_version_missing
    translator_library/validate/wizard_delete_version_gap
    translator_library/validate/wizard_collapse_version_missing
    translator_library/validate/wizard_collapse_version_owner_missing
    translator_library/validate/wizard_collapse_version_range
    translator_library/validate/anonymous_without_owner
    translator_library/validate/anonymous_owner_deleted
    translator_library/validate/anonymous_not_before
    translator_library/validate/wizard_create_tag_not_context
    translator_library/validate/wizard_create_tag_not_named
    translator_library/validate/wizard_create_tag_named_in_version
    translator_library/validate/wizard_create_tag_missing
    translator_library/validate/wizard_create_tag_closed
    translator_library/validate/wizard_create_tag_deleted
    translator_library/validate/wizard_create_tag_not_open
    translator_library/validate/wizard_create_ordering_not_root
    translator_library/validate/wizard_create_ordering_not_context
    translator_library/validate/wizard_create_aggregate_not_chunk
    translator_library/validate/wizard_create_aggregate_names_repeated
    translator_library/validate/wizard_create_aggregate_command_file_list
    translator_library/validate/wizard_create_aggregate_list_chunk_exists
    translator_library/validate/wizard_create_aggregate_chunk_exists
    translator_library/validate/wizard_create_aggregate_closed
    translator_library/validate/hash_match
    translator_library/validate/hash_mismatch
    translator_library/validate/hash_force_other
    translator_library/validate/hash_absent
    translator_library/validate/names_duplicate
    translator_library/validate/label_missing
    translator_library/validate/label_later
    translator_library/validate/label_present
    translator_library/validate/delete_label
    translator_library/cpp2post_cpp/delete_tag/same_version
    translator_library/cpp2post_cpp/delete_tag/later_version
    translator_library/cpp2post_cpp/delete_tag/redeclare
    translator_library/cpp2post_cpp/delete_tag/absent
    translator_library/cpp2post_cpp/delete_version/strip
    translator_library/cpp2post_cpp/ordering/later_wins
    translator_library/cpp2post_cpp/ordering/unknown
    translator_library/cpp2post_cpp/ordering/twice
    translator_library/cpp2post_cpp/ordering/cycle
    translator_library/cpp2post_cpp/ordering/not_weak
    translator_library/cpp2post_cpp/ordering/absent
    translator_library/cpp2post_cpp/ordering/partial
    translator_library/cpp2post_cpp/ordering/tie
    translator_library/cpp2post_cpp/ordering/chain
    translator_library/cpp2post_cpp/ordering/diamond
    translator_library/cpp2post_cpp/ordering/fork
    translator_library/cpp2post_cpp/naming/from_label
    translator_library/cpp2post_cpp/naming/no_namer
    translator_library/cpp2post_cpp/naming/namer_not_in_values
    translator_library/cpp2post_cpp/naming/no_value
    translator_library/cpp2post_cpp/naming/not_identifier
    translator_library/cpp2post_cpp/naming/different_names
    translator_library/cpp2post_cpp/label/duplicate_value
    translator_library/cpp2post_cpp/label/important
    translator_library/cpp2post_cpp/tag_mode
    translator_library/cpp2post_cpp/version_mode
    translator_library/cpp2post_cpp/version_own
    translator_library/validate/root_version_local
    translator_library/validate/root_version_own
)

foreach(case IN LISTS galimatron_cases_with_common_gal)
    message(STATUS "copy common gal.cfg and log.cfg to ${galimatron_test_data_dir}/${case}")
    file(COPY
        ${galimatron_default_cfg}/gal.cfg
        ${galimatron_default_cfg}/log.cfg
        DESTINATION ${galimatron_test_data_dir}/${case}
    )
    message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/${case}/xslt")
    file(COPY
        ${galimatron_default_cfg}/xslt/config.xml
        DESTINATION ${galimatron_test_data_dir}/${case}/xslt
    )
endforeach()

foreach(case IN ITEMS reassign inherit open present unchanged)
    message(STATUS "copy seal gal.cfg and log.cfg to ${galimatron_test_data_dir}/translator_library/xslt_seal/${case}")
    file(COPY
        ${galimatron_default_cfg}/log.cfg
        ${galimatron_common_cfg}/xslt_seal/gal.cfg
        DESTINATION ${galimatron_test_data_dir}/translator_library/xslt_seal/${case}
    )
    message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/xslt_seal/${case}/xslt")
    file(COPY
        ${galimatron_default_cfg}/xslt/config.xml
        DESTINATION ${galimatron_test_data_dir}/translator_library/xslt_seal/${case}/xslt
    )
endforeach()

foreach(case IN ITEMS reassign inherit open)
    message(STATUS "copy seal edit.xsl and gal-functions.xsl to ${galimatron_test_data_dir}/translator_library/xslt_seal/${case}/xslt")
    file(COPY
        ${galimatron_common_cfg}/xslt_seal/edit.xsl
        ${CMAKE_CURRENT_SOURCE_DIR}/../translator_library/xslt/gal-functions.xsl
        DESTINATION ${galimatron_test_data_dir}/translator_library/xslt_seal/${case}/xslt
    )
endforeach()

message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/xslt_catalog/xslt")
file(COPY
    ${galimatron_default_cfg}/xslt/config.xml
    DESTINATION ${galimatron_test_data_dir}/translator_library/xslt_catalog/xslt
)

message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/xslt_catalog_resolved/xslt")
file(COPY
    ${galimatron_default_cfg}/xslt/config.xml
    DESTINATION ${galimatron_test_data_dir}/translator_library/xslt_catalog_resolved/xslt
)

message(STATUS "copy common log.cfg to ${galimatron_test_data_dir}/translator_library/cpp2gal/template_engine_xslt")
file(COPY
    ${galimatron_default_cfg}/log.cfg
    DESTINATION ${galimatron_test_data_dir}/translator_library/cpp2gal/template_engine_xslt
)
message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/cpp2gal/template_engine_xslt/xslt")
file(COPY
    ${galimatron_default_cfg}/xslt/config.xml
    DESTINATION ${galimatron_test_data_dir}/translator_library/cpp2gal/template_engine_xslt/xslt
)

foreach(case IN ITEMS dynamic)
    message(STATUS "copy common log.cfg to ${galimatron_test_data_dir}/translator_library/wizard_create_version/${case}")
    file(COPY
        ${galimatron_default_cfg}/log.cfg
        DESTINATION ${galimatron_test_data_dir}/translator_library/wizard_create_version/${case}
    )
    message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/wizard_create_version/${case}/xslt")
    file(COPY
        ${galimatron_default_cfg}/xslt/config.xml
        DESTINATION ${galimatron_test_data_dir}/translator_library/wizard_create_version/${case}/xslt
    )
endforeach()

message(STATUS "copy common log.cfg to ${galimatron_test_data_dir}/translator_library/normal_mode")
file(COPY
    ${galimatron_default_cfg}/log.cfg
    DESTINATION ${galimatron_test_data_dir}/translator_library/normal_mode
)
message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/normal_mode/xslt")
file(COPY
    ${galimatron_default_cfg}/xslt/config.xml
    DESTINATION ${galimatron_test_data_dir}/translator_library/normal_mode/xslt
)

message(STATUS "copy common log.cfg to ${galimatron_test_data_dir}/translator_library/uuid_protocol")
file(COPY
    ${galimatron_default_cfg}/log.cfg
    DESTINATION ${galimatron_test_data_dir}/translator_library/uuid_protocol
)
message(STATUS "copy common xslt/config.xml to ${galimatron_test_data_dir}/translator_library/uuid_protocol/xslt")
file(COPY
    ${galimatron_default_cfg}/xslt/config.xml
    DESTINATION ${galimatron_test_data_dir}/translator_library/uuid_protocol/xslt
)
