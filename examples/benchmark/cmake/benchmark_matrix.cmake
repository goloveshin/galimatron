# dynamic, the default: 32 runs. aggregates 10, 20, 50, and 100.
# versions 10, 20, 50, and 100. dynamic payload tags 2 and 8.
# static, BENCHMARK_MATRIX_KIND=static: 12 runs. aggregates 10, 20, 50, and 500.
# version count 10. static payload tags 1, 50, and 100. those tags stay in version 0.
# static-no-xslt uses the same grid and gal-static-no-xslt.cfg, which has no user xslt.
# each cell rewrites incoming_gal, runs Galimatron, and keeps that log.
# the last step restores the current benchmark defaults: 500 aggregates, 10 versions, 2 dynamic tags.
# GALIMATRON_EXECUTABLE is the built galimatron.exe.
# BENCHMARK_SOURCE_DIR is examples/benchmark.
#
if(NOT DEFINED GALIMATRON_EXECUTABLE)
    message(FATAL_ERROR "GALIMATRON_EXECUTABLE is not set")
endif()
if(NOT DEFINED BENCHMARK_SOURCE_DIR)
    message(FATAL_ERROR "BENCHMARK_SOURCE_DIR is not set")
endif()

set(benchmark_incoming "${BENCHMARK_SOURCE_DIR}/incoming_gal")
set(benchmark_protocol "${benchmark_incoming}/protocol.gal")
set(benchmark_console "${benchmark_incoming}/console.gal")
set(benchmark_generator "${BENCHMARK_SOURCE_DIR}/cmake/create_initial_protocol.cmake")
set(benchmark_matrix_dir "${BENCHMARK_SOURCE_DIR}/matrix")
if(NOT DEFINED BENCHMARK_MATRIX_KIND)
    set(BENCHMARK_MATRIX_KIND dynamic)
endif()
if(BENCHMARK_MATRIX_KIND STREQUAL "static" OR BENCHMARK_MATRIX_KIND STREQUAL "static-no-xslt")
    set(benchmark_aggregates 10 20 50 500)
    set(benchmark_versions 10)
    set(benchmark_tags 1 50 100)
    set(benchmark_tag_kind static)
    if(BENCHMARK_MATRIX_KIND STREQUAL "static-no-xslt")
        set(benchmark_log_dir "${benchmark_matrix_dir}/logs-static-no-xslt")
        set(benchmark_table "${benchmark_matrix_dir}/times-static-no-xslt.md")
        set(benchmark_gal_cfg "${BENCHMARK_SOURCE_DIR}/gal-static-no-xslt.cfg")
    else()
        set(benchmark_log_dir "${benchmark_matrix_dir}/logs-static")
        set(benchmark_table "${benchmark_matrix_dir}/times-static.md")
    endif()
elseif(BENCHMARK_MATRIX_KIND STREQUAL "dynamic")
    set(benchmark_aggregates 10 20 50 100)
    set(benchmark_versions 10 20 50 100)
    set(benchmark_tags 2 8)
    set(benchmark_tag_kind dynamic)
    set(benchmark_log_dir "${benchmark_matrix_dir}/logs")
    set(benchmark_table "${benchmark_matrix_dir}/times.md")
else()
    message(FATAL_ERROR
        "BENCHMARK_MATRIX_KIND must be dynamic, static, or static-no-xslt, got '${BENCHMARK_MATRIX_KIND}'"
    )
endif()
if(NOT DEFINED benchmark_gal_cfg)
    set(benchmark_gal_cfg "${BENCHMARK_SOURCE_DIR}/gal.cfg")
endif()
file(MAKE_DIRECTORY "${benchmark_log_dir}")

file(WRITE "${benchmark_table}"
"| agg | ver | tags | total s | wizard s | cpp2xml s | xslt s | xml2cpp s | cpp2gal s | post xml s | state |\n"
)
file(APPEND "${benchmark_table}"
"| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |\n"
)

function(benchmark_seconds ms out_text)
    if(ms STREQUAL "")
        set(${out_text} "-" PARENT_SCOPE)
        return()
    endif()
    math(EXPR whole "${ms} / 1000")
    math(EXPR tenth "(${ms} % 1000) / 100")
    set(${out_text} "${whole}.${tenth}" PARENT_SCOPE)
endfunction()

function(benchmark_generate aggregate_count version_count tag_count)
    if(ARGC GREATER 3)
        set(tag_kind "${ARGV3}")
    else()
        set(tag_kind "${benchmark_tag_kind}")
    endif()
    execute_process(
        COMMAND
            ${CMAKE_COMMAND}
                -DBENCHMARK_AGGREGATE_COUNT=${aggregate_count}
                -DBENCHMARK_VERSION_COUNT=${version_count}
                -DBENCHMARK_TAG_COUNT=${tag_count}
                -DBENCHMARK_TAG_KIND=${tag_kind}
                -DBENCHMARK_INCOMING_DIR=${benchmark_incoming}
                -DBENCHMARK_PROTOCOL_GAL=${benchmark_protocol}
                -DBENCHMARK_CONSOLE_GAL=${benchmark_console}
                -P ${benchmark_generator}
        RESULT_VARIABLE generate_result
    )
    if(NOT generate_result EQUAL 0)
        message(FATAL_ERROR
            "benchmark generate ${aggregate_count}/${version_count}/${tag_count} failed: ${generate_result}"
        )
    endif()
endfunction()

function(benchmark_read_log log_path out_row)
    file(STRINGS "${log_path}" lines)
    set(total "")
    set(wizard "")
    set(cpp2xml "")
    set(xslt "")
    set(xml2cpp "")
    set(cpp2gal "")
    set(post_xml "")
    set(state "")
    foreach(line IN LISTS lines)
        if(line MATCHES "end of Galimatron:.*\\(([0-9]+) ms\\)")
            set(total "${CMAKE_MATCH_1}")
        elseif(line MATCHES "all state: ([0-9.]+ [MG]B)")
            set(state "${CMAKE_MATCH_1}")
        elseif(line MATCHES "all state: ([0-9]+) bytes")
            set(state "${CMAKE_MATCH_1} bytes")
        elseif(line MATCHES "end +of transformation: [0-9]+, '([^']+)':.*\\(([0-9]+) ms\\)")
            set(step "${CMAKE_MATCH_1}")
            set(step_ms "${CMAKE_MATCH_2}")
            if(step STREQUAL "cpp2cpp_wizard")
                set(wizard "${step_ms}")
            elseif(step STREQUAL "cpp2xml")
                set(cpp2xml "${step_ms}")
            elseif(step STREQUAL "suffix fresh tags")
                set(xslt "${step_ms}")
            elseif(step STREQUAL "xml2cpp")
                set(xml2cpp "${step_ms}")
            elseif(step STREQUAL "cpp2gal")
                set(cpp2gal "${step_ms}")
            elseif(step STREQUAL "post_cpp2post_xml")
                set(post_xml "${step_ms}")
            endif()
        endif()
    endforeach()
    benchmark_seconds("${total}" total_s)
    benchmark_seconds("${wizard}" wizard_s)
    benchmark_seconds("${cpp2xml}" cpp2xml_s)
    benchmark_seconds("${xslt}" xslt_s)
    benchmark_seconds("${xml2cpp}" xml2cpp_s)
    benchmark_seconds("${cpp2gal}" cpp2gal_s)
    benchmark_seconds("${post_xml}" post_xml_s)
    if(state STREQUAL "")
        set(state "-")
    endif()
    set(${out_row}
        "| ${benchmark_cell_agg} | ${benchmark_cell_ver} | ${benchmark_cell_tags} | ${total_s} | ${wizard_s} | ${cpp2xml_s} | ${xslt_s} | ${xml2cpp_s} | ${cpp2gal_s} | ${post_xml_s} | ${state} |"
        PARENT_SCOPE
    )
endfunction()

function(benchmark_one aggregate_count version_count tag_count)
    message(STATUS "benchmark cell ${aggregate_count} aggregates, ${version_count} versions, ${tag_count} ${benchmark_tag_kind} tags")
    benchmark_generate("${aggregate_count}" "${version_count}" "${tag_count}")
    file(REMOVE_RECURSE
        "${BENCHMARK_SOURCE_DIR}/intermediate"
        "${BENCHMARK_SOURCE_DIR}/actual_gal"
    )
    execute_process(
        COMMAND
            ${GALIMATRON_EXECUTABLE}
                --gal-config-file ${benchmark_gal_cfg}
                --log-config-file ${BENCHMARK_SOURCE_DIR}/log.cfg
        WORKING_DIRECTORY ${BENCHMARK_SOURCE_DIR}
        RESULT_VARIABLE run_result
    )
    set(saved_log "${benchmark_log_dir}/a${aggregate_count}-v${version_count}-t${tag_count}.log")
    if(EXISTS "${BENCHMARK_SOURCE_DIR}/intermediate/galimatron.log")
        file(COPY_FILE
            "${BENCHMARK_SOURCE_DIR}/intermediate/galimatron.log"
            "${saved_log}"
        )
    endif()
    if(NOT run_result EQUAL 0)
        benchmark_generate(500 10 2 dynamic)
        message(FATAL_ERROR
            "benchmark ${aggregate_count}/${version_count}/${tag_count} failed: ${run_result}"
        )
    endif()
    if(NOT EXISTS "${saved_log}")
        benchmark_generate(500 10 2 dynamic)
        message(FATAL_ERROR "benchmark log was not written: ${saved_log}")
    endif()
    set(benchmark_cell_agg "${aggregate_count}")
    set(benchmark_cell_ver "${version_count}")
    set(benchmark_cell_tags "${tag_count}")
    benchmark_read_log("${saved_log}" row)
    file(APPEND "${benchmark_table}" "${row}\n")
    message(STATUS "${row}")
endfunction()

foreach(aggregate_count IN LISTS benchmark_aggregates)
    foreach(version_count IN LISTS benchmark_versions)
        foreach(tag_count IN LISTS benchmark_tags)
            benchmark_one("${aggregate_count}" "${version_count}" "${tag_count}")
        endforeach()
    endforeach()
endforeach()

benchmark_generate(500 10 2 dynamic)
file(READ "${benchmark_table}" benchmark_table_text)
message(STATUS "benchmark matrix\n${benchmark_table_text}")
