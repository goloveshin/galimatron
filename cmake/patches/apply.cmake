if(NOT DEFINED SOURCE_DIR OR NOT DEFINED PATCH_FILE)
    message(FATAL_ERROR "SOURCE_DIR and PATCH_FILE are required")
endif()

if(NOT EXISTS "${SOURCE_DIR}")
    message(FATAL_ERROR "SOURCE_DIR does not exist: ${SOURCE_DIR}")
endif()

if(NOT EXISTS "${PATCH_FILE}")
    message(FATAL_ERROR "PATCH_FILE does not exist: ${PATCH_FILE}")
endif()

# A Windows checkout stores the patch with CR LF. git apply then rejects it
# against a dependency that stores LF. doctest sets eol=lf.
# file(WRITE) on Windows would put the CR LF back. file(CONFIGURE) can force LF.
# file(CONFIGURE) also substitutes @VAR@, so a real @ is hidden and put back.
file(READ "${PATCH_FILE}" patch_text)
string(REPLACE "\r" "" patch_text "${patch_text}")
string(REPLACE "@" "@GALIMATRON_PATCH_AT@" patch_text "${patch_text}")
set(GALIMATRON_PATCH_AT "@")

string(RANDOM LENGTH 12 ALPHABET "0123456789abcdef" patch_suffix)
if(DEFINED ENV{TEMP} AND NOT "$ENV{TEMP}" STREQUAL "")
    set(normalized_patch "$ENV{TEMP}/galimatron-${patch_suffix}.patch")
elseif(DEFINED ENV{TMPDIR} AND NOT "$ENV{TMPDIR}" STREQUAL "")
    set(normalized_patch "$ENV{TMPDIR}/galimatron-${patch_suffix}.patch")
else()
    set(normalized_patch "${SOURCE_DIR}/.galimatron-${patch_suffix}.patch")
endif()
file(CONFIGURE
    OUTPUT "${normalized_patch}"
    CONTENT "${patch_text}"
    @ONLY
    NEWLINE_STYLE LF
)

execute_process(
    COMMAND git apply --check --reverse "${normalized_patch}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE already_applied
    OUTPUT_QUIET
    ERROR_QUIET
)

if(already_applied EQUAL 0)
    file(REMOVE "${normalized_patch}")
    return()
endif()

execute_process(
    COMMAND git apply "${normalized_patch}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE apply_result
    ERROR_VARIABLE apply_error
)

file(REMOVE "${normalized_patch}")

if(NOT apply_result EQUAL 0)
    message(FATAL_ERROR "Failed to apply ${PATCH_FILE} in ${SOURCE_DIR}:\n${apply_error}")
endif()
