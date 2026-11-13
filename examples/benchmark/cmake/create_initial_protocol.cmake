# each aggregate is a real file: version 0 holds the statement pack.
# the wizard adds the later versions, and on each of them rewrites one tag, drops one, and adds one.
# the last command hashes version 0, so the language xslt has a sealed version to restore.
# BENCHMARK_AGGREGATE_COUNT is the number of aggregates.
# BENCHMARK_VERSION_COUNT is the number of versions, including version 0.
# BENCHMARK_TAG_COUNT is the number of payload tags.
# BENCHMARK_TAG_KIND is dynamic or static. dynamic copies those tags into each new version.
# static leaves them in version 0. absent means dynamic.
# BENCHMARK_INCOMING_DIR is the source folder: protocol.gal, console.gal, and the aggregate files.
# BENCHMARK_CONSOLE_GAL is the wizard file.
# BENCHMARK_PROTOCOL_GAL is the root aggregate file.
#
if(NOT DEFINED BENCHMARK_AGGREGATE_COUNT)
    message(FATAL_ERROR "BENCHMARK_AGGREGATE_COUNT is not set")
endif()
if(NOT BENCHMARK_AGGREGATE_COUNT MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR
        "BENCHMARK_AGGREGATE_COUNT must be a positive integer, got '${BENCHMARK_AGGREGATE_COUNT}'"
    )
endif()
if(NOT DEFINED BENCHMARK_VERSION_COUNT)
    message(FATAL_ERROR "BENCHMARK_VERSION_COUNT is not set")
endif()
if(NOT BENCHMARK_VERSION_COUNT MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR
        "BENCHMARK_VERSION_COUNT must be a positive integer, got '${BENCHMARK_VERSION_COUNT}'"
    )
endif()
if(NOT DEFINED BENCHMARK_TAG_COUNT)
    message(FATAL_ERROR "BENCHMARK_TAG_COUNT is not set")
endif()
if(NOT BENCHMARK_TAG_COUNT MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR
        "BENCHMARK_TAG_COUNT must be a positive integer, got '${BENCHMARK_TAG_COUNT}'"
    )
endif()
if(NOT DEFINED BENCHMARK_TAG_KIND)
    set(BENCHMARK_TAG_KIND dynamic)
endif()
if(NOT BENCHMARK_TAG_KIND MATCHES "^(dynamic|static)$")
    message(FATAL_ERROR
        "BENCHMARK_TAG_KIND must be dynamic or static, got '${BENCHMARK_TAG_KIND}'"
    )
endif()
if(NOT DEFINED BENCHMARK_INCOMING_DIR)
    message(FATAL_ERROR "BENCHMARK_INCOMING_DIR is not set")
endif()
if(NOT DEFINED BENCHMARK_PROTOCOL_GAL)
    message(FATAL_ERROR "BENCHMARK_PROTOCOL_GAL is not set")
endif()
if(NOT DEFINED BENCHMARK_CONSOLE_GAL)
    message(FATAL_ERROR "BENCHMARK_CONSOLE_GAL is not set")
endif()

file(MAKE_DIRECTORY "${BENCHMARK_INCOMING_DIR}")

function(benchmark_pad_name index width out_name)
    set(padded "${index}")
    string(LENGTH "${padded}" padded_length)
    while(padded_length LESS width)
        set(padded "0${padded}")
        math(EXPR padded_length "${padded_length} + 1")
    endwhile()
    set(${out_name} "${padded}" PARENT_SCOPE)
endfunction()

function(benchmark_statement_pack title out_pack)
    set(pack "")
    if(BENCHMARK_TAG_KIND STREQUAL "dynamic")
        string(APPEND pack "    label marker = dynamic { fresh };\n")
    else()
        string(APPEND pack "    label text = static { text };\n")
    endif()
    string(APPEND pack "    label field = static { type, name }, tag-name-from { name };\n")
    string(APPEND pack "    title = important { \"${title}\" };\n")
    if(BENCHMARK_TAG_KIND STREQUAL "dynamic")
        string(APPEND pack "    note = marker { \"benchmark\" };\n")
    endif()
    string(APPEND pack "    bulk = { \"${benchmark_bulk}\" };\n")
    string(APPEND pack "    field { \"text\", \"speed\" };\n")
    string(APPEND pack "    rendered = { \"see {{title}}\"_te };\n")
    set(tag_index 0)
    while(tag_index LESS BENCHMARK_TAG_COUNT)
        if(BENCHMARK_TAG_KIND STREQUAL "dynamic")
            string(APPEND pack "    payload_${tag_index} = marker { \"payload\" };\n")
        else()
            string(APPEND pack "    payload_${tag_index} = text { \"payload\" };\n")
        endif()
        math(EXPR tag_index "${tag_index} + 1")
    endwhile()
    set(${out_pack} "${pack}" PARENT_SCOPE)
endfunction()

function(benchmark_write_if_changed path content)
    if(EXISTS "${path}")
        file(READ "${path}" existing)
    else()
        set(existing "")
    endif()
    if(NOT existing STREQUAL content)
        file(WRITE "${path}" "${content}")
    endif()
endfunction()

math(EXPR benchmark_last_index "${BENCHMARK_AGGREGATE_COUNT} - 1")
string(LENGTH "${benchmark_last_index}" benchmark_name_width)
if(benchmark_name_width LESS 2)
    set(benchmark_name_width 2)
endif()

string(REPEAT "benchmark " 40 benchmark_bulk)

set(benchmark_item_names "")
set(benchmark_index 0)
while(benchmark_index LESS BENCHMARK_AGGREGATE_COUNT)
    benchmark_pad_name("${benchmark_index}" "${benchmark_name_width}" benchmark_item_suffix)
    list(APPEND benchmark_item_names "Item${benchmark_item_suffix}")
    math(EXPR benchmark_index "${benchmark_index} + 1")
endwhile()

list(JOIN benchmark_item_names ", " benchmark_ordering_names)

set(benchmark_kept_items "")
foreach(benchmark_item_name IN LISTS benchmark_item_names)
    benchmark_statement_pack("${benchmark_item_name}" benchmark_item_pack)
    set(benchmark_item_gal
"aggregate ${benchmark_item_name}
{
    versions
    {
        0;
    }
}

version 0
{
${benchmark_item_pack}}
")
    set(benchmark_item_path "${BENCHMARK_INCOMING_DIR}/${benchmark_item_name}.gal")
    benchmark_write_if_changed("${benchmark_item_path}" "${benchmark_item_gal}")
    list(APPEND benchmark_kept_items "${benchmark_item_path}")
endforeach()

file(GLOB benchmark_existing_items "${BENCHMARK_INCOMING_DIR}/Item*.gal")
foreach(benchmark_existing_item IN LISTS benchmark_existing_items)
    if(NOT benchmark_existing_item IN_LIST benchmark_kept_items)
        file(REMOVE "${benchmark_existing_item}")
    endif()
endforeach()

benchmark_statement_pack("Protocol" benchmark_root_pack)
set(benchmark_protocol
"root aggregate Protocol
{
    versions
    {
        0;
    }
}

version 0
{
${benchmark_root_pack}    ordering listed = { ${benchmark_ordering_names} };
}
")
benchmark_write_if_changed("${BENCHMARK_PROTOCOL_GAL}" "${benchmark_protocol}")

set(benchmark_edit_targets "Protocol")
list(APPEND benchmark_edit_targets ${benchmark_item_names})

math(EXPR benchmark_extra_versions "${BENCHMARK_VERSION_COUNT} - 1")
if(BENCHMARK_TAG_KIND STREQUAL "dynamic")
    set(benchmark_payload_label "marker")
else()
    set(benchmark_payload_label "text")
endif()
set(benchmark_gal
"// ${BENCHMARK_AGGREGATE_COUNT} aggregates, ${BENCHMARK_VERSION_COUNT} versions, ${BENCHMARK_TAG_COUNT} ${BENCHMARK_TAG_KIND} tags.
//
wizard {
")
set(benchmark_index 0)
while(benchmark_index LESS benchmark_extra_versions)
    string(APPEND benchmark_gal "    create version;\n")
    string(APPEND benchmark_gal "    create ordering Protocol listed = { ${benchmark_ordering_names} };\n")
    foreach(benchmark_edit_target IN LISTS benchmark_edit_targets)
        string(APPEND benchmark_gal "    delete tag ${benchmark_edit_target} payload_0;\n")
        string(APPEND benchmark_gal "    create tag ${benchmark_edit_target} payload_0 = ${benchmark_payload_label} { \"back\" };\n")
        string(APPEND benchmark_gal "    create tag ${benchmark_edit_target} added_${benchmark_index} = { \"new\" };\n")
    endforeach()
    math(EXPR benchmark_index "${benchmark_index} + 1")
endwhile()
string(APPEND benchmark_gal "    create hash 0;\n")
string(APPEND benchmark_gal "}\n")
benchmark_write_if_changed("${BENCHMARK_CONSOLE_GAL}" "${benchmark_gal}")
