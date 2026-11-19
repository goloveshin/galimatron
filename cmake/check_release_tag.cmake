# the release tag must be v plus project() VERSION.
# CPack names the packages from that same version.
set(tag "$ENV{GALIMATRON_RELEASE_TAG}")
set(cache_file "$ENV{GALIMATRON_CACHE_FILE}")
if(tag STREQUAL "" OR cache_file STREQUAL "")
    message(FATAL_ERROR "GALIMATRON_RELEASE_TAG and GALIMATRON_CACHE_FILE must be set")
endif()

file(STRINGS "${cache_file}" version_lines REGEX "^CMAKE_PROJECT_VERSION:STATIC=")
list(LENGTH version_lines version_count)
if(NOT version_count EQUAL 1)
    message(FATAL_ERROR "CMAKE_PROJECT_VERSION is missing from ${cache_file}")
endif()

list(GET version_lines 0 version_line)
string(REGEX REPLACE "^CMAKE_PROJECT_VERSION:STATIC=" "" project_version "${version_line}")

set(expected_tag "v${project_version}")
if(NOT tag STREQUAL expected_tag)
    message(FATAL_ERROR
        "tag ${tag} does not match project version ${project_version}. expected ${expected_tag}"
    )
endif()

message(STATUS "release tag ${tag} matches project version ${project_version}")
