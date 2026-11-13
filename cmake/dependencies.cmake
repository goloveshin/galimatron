cmake_minimum_required(VERSION 4.0)

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

set(galimatron_apply_patch ${CMAKE_CURRENT_LIST_DIR}/patches/apply.cmake)

function(galimatron_patch_dependency dependency patch_file)
    # FetchContent records POPULATED only after MakeAvailable in this configure.
    # An already downloaded tree is patched here; a first download is patched
    # by PATCH_COMMAND before the dependency is configured.
    # A third argument is a directory under the dependency source.
    string(TOLOWER "${dependency}" dependency_lower)
    set(source_dir "${CMAKE_BINARY_DIR}/_deps/${dependency_lower}-src")
    if(ARGC GREATER 2)
        set(source_dir "${source_dir}/${ARGV2}")
    endif()
    if(IS_DIRECTORY "${source_dir}")
        execute_process(
            COMMAND ${CMAKE_COMMAND}
                -DSOURCE_DIR=${source_dir}
                -DPATCH_FILE=${patch_file}
                -P ${galimatron_apply_patch}
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()
endfunction()

# Boost

set(BOOST_RUNTIME_LINK shared)

# CMake 4.5 requires a source_group(TREE) root to be a directory prefix.
# Boost 1.92 groups include/boost/locale.hpp under include/boost/locale.
# Boost.Locale after 1.92 drops that header from the tree group.
set(galimatron_boost_locale_patch ${CMAKE_CURRENT_LIST_DIR}/patches/boost-locale-source-group.patch)

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.92.0
    EXCLUDE_FROM_ALL
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR>/libs/locale -DPATCH_FILE=${galimatron_boost_locale_patch} -P ${galimatron_apply_patch}
    FIND_PACKAGE_ARGS
)
galimatron_patch_dependency(Boost ${galimatron_boost_locale_patch} libs/locale)
FetchContent_MakeAvailable(Boost)

# logme
# LoadConfiguration() parses JSON only when logme is built with JsonCpp.

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_OBJECT_LIBS OFF CACHE BOOL "" FORCE)

# The generated config below only checks that JsonCpp::JsonCpp already exists.
# It must not satisfy find_package(jsoncpp) inside FetchContent_MakeAvailable().
set(_jsoncpp_config_dir "${CMAKE_BINARY_DIR}/generated/jsoncpp")
if(jsoncpp_DIR STREQUAL "${_jsoncpp_config_dir}")
    unset(jsoncpp_DIR CACHE)
endif()

FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG        1.9.6
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(jsoncpp)

if(NOT TARGET JsonCpp::JsonCpp)
    if(TARGET jsoncpp_static)
        add_library(JsonCpp::JsonCpp ALIAS jsoncpp_static)
    elseif(TARGET jsoncpp_lib)
        add_library(JsonCpp::JsonCpp ALIAS jsoncpp_lib)
    endif()
endif()

file(WRITE "${_jsoncpp_config_dir}/jsoncppConfig.cmake"
"if(NOT TARGET JsonCpp::JsonCpp)\n  message(FATAL_ERROR \"JsonCpp::JsonCpp was not created\")\nendif()\n"
)
set(jsoncpp_DIR "${_jsoncpp_config_dir}" CACHE PATH "" FORCE)

set(LOGME_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LOGME_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LOGME_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(LOGME_BUILD_DYNAMIC OFF CACHE BOOL "" FORCE)
set(LOGME_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(LOGME_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(USE_JSONCPP ON CACHE STRING "" FORCE)
set(USE_ZLIB OFF CACHE STRING "" FORCE)

FetchContent_Declare(
    logme
    GIT_REPOSITORY https://github.com/efmsoft/logme.git
    GIT_TAG        main
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(logme)

# logme's CMakeLists sets every output directory to CMAKE_BINARY_DIR.
# A static library uses the archive directory, so logme.lib lands in the build root.
set_target_properties(logme PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# range-v3

FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(range-v3)

# Antlr4

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/antlr-4.13.2-complete.jar)
    message(STATUS "Downloading antlr-4.13.2-complete.jar")

    file(DOWNLOAD
        https://www.antlr.org/download/antlr-4.13.2-complete.jar
        ${CMAKE_CURRENT_BINARY_DIR}/antlr-4.13.2-complete.jar
        SHOW_PROGRESS
    )
endif()

set(ANTLR4_JAR_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/antlr-4.13.2-complete.jar)

set(ANTLR4_INSTALL TRUE)

if (MSVC)
    # static ANTLR library (no DLL); the CRT stays dynamic, as in the project
    add_compile_definitions(ANTLR4CPP_STATIC)
    set(WITH_STATIC_CRT OFF CACHE BOOL "Use the dynamic MSVC CRT for the ANTLR runtime" FORCE)
endif()

# ANTLR runtime tests copy the .lib into ${CMAKE_HOME_DIRECTORY}/dist after the build,
# and that directory is the project source root
set(ANTLR_BUILD_CPP_TESTS OFF)

set(galimatron_antlr4_patch ${CMAKE_CURRENT_LIST_DIR}/patches/antlr4-profiling-chrono.patch)

FetchContent_Declare(
    antlr4_runtime
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        4.13.2
    SOURCE_SUBDIR  runtime/Cpp
    EXCLUDE_FROM_ALL
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -DPATCH_FILE=${galimatron_antlr4_patch} -P ${galimatron_apply_patch}
    FIND_PACKAGE_ARGS
)
galimatron_patch_dependency(antlr4_runtime ${galimatron_antlr4_patch})
FetchContent_MakeAvailable(antlr4_runtime)

FetchContent_GetProperties(antlr4_runtime
    BINARY_DIR antlr4_runtime_BINARY_DIR
)

find_package(antlr4-generator
    PATHS ${antlr4_runtime_BINARY_DIR}
    REQUIRED
)

# PugiXML

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(pugixml)

add_compile_definitions(PUGIXML_NO_XPATH)

# nlohmann_json

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(nlohmann_json)

# inja. the upstream CMakeLists runs a shell script, so a stand-in list is written after download

set(galimatron_inja_cmake ${CMAKE_CURRENT_LIST_DIR}/patches/inja_cmake.cmake)

FetchContent_Declare(
    inja
    GIT_REPOSITORY https://github.com/pantor/inja.git
    GIT_TAG v3.4.0
    EXCLUDE_FROM_ALL
    PATCH_COMMAND ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR> -P ${galimatron_inja_cmake}
)
FetchContent_MakeAvailable(inja)

# alpaca serialization

FetchContent_Declare(
    alpaca
    GIT_REPOSITORY https://github.com/p-ranav/alpaca.git
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(alpaca)

# doctest

set(galimatron_doctest_patch ${CMAKE_CURRENT_LIST_DIR}/patches/doctest-cmake-4.0.patch)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    EXCLUDE_FROM_ALL
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -DPATCH_FILE=${galimatron_doctest_patch} -P ${galimatron_apply_patch}
    FIND_PACKAGE_ARGS
)
galimatron_patch_dependency(doctest ${galimatron_doctest_patch})
FetchContent_MakeAvailable(doctest)

# magic_enum

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(magic_enum)

# Taskflow (header-only). The translator runs one sequential chain.

set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(TF_BUILD_PROFILER OFF CACHE BOOL "" FORCE)
set(TF_BUILD_CUDA OFF CACHE BOOL "" FORCE)
set(TF_BUILD_MODULES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    Taskflow
    GIT_REPOSITORY https://github.com/taskflow/taskflow
    GIT_TAG        v4.0.0
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Taskflow)

# Saxon (SaxonC-HE) processor from Saxonica (https://www.saxonica.com)

if(WIN32)
    set(_saxon_url ${CMAKE_SOURCE_DIR}/externals/Saxon/SaxonCHE-windows-x86_64-13-0-0.zip)
else()
    set(_saxon_url ${CMAKE_SOURCE_DIR}/externals/Saxon/SaxonCHE-linux-x86_64-13-0-0.zip)
endif()

FetchContent_Declare(
    Saxon
    URL ${_saxon_url}
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Saxon)

# The zip may extract flat, or under one or two top folders named after the archive.
file(GLOB _saxon_core_dirs
    "${saxon_SOURCE_DIR}/SaxonCCoreHE"
    "${saxon_SOURCE_DIR}/*/SaxonCCoreHE"
    "${saxon_SOURCE_DIR}/*/*/SaxonCCoreHE"
)
list(LENGTH _saxon_core_dirs _saxon_core_count)
if(NOT _saxon_core_count EQUAL 1)
    message(FATAL_ERROR "SaxonCCoreHE not found once in ${saxon_SOURCE_DIR}")
endif()
cmake_path(GET _saxon_core_dirs PARENT_PATH _saxon_root)

# 13.0 lists include/saxonc-core_export.h in the imported FILE_SET, but the
# zip does not ship that file. Nothing includes it.
set(_saxon_core_cmake "${_saxon_root}/SaxonCCoreHE/lib/cmake/SaxonCCoreHE/SaxonCCoreHE.cmake")
file(READ "${_saxon_core_cmake}" _saxon_core_cmake_text)
string(REGEX REPLACE
    "\r?\n    INTERFACE\r?\n      FILE_SET \"export_header\"\r?\n      TYPE \"HEADERS\"\r?\n      BASE_DIRS \"\\$\\{_IMPORT_PREFIX}/include\"\r?\n      FILES \"\\$\\{_IMPORT_PREFIX}/include/saxonc-core_export.h\""
    ""
    _saxon_core_cmake_text "${_saxon_core_cmake_text}"
)
file(WRITE "${_saxon_core_cmake}" "${_saxon_core_cmake_text}")

# The zip root has no CMakeLists.txt. Populate it, import SaxonCCoreHE in this
# directory (so later subdirectories can see it), then add src/.
# The package ships only a Release import. A Debug build uses that binary.
set(SaxonCCoreHE_DIR "${_saxon_root}/SaxonCCoreHE/lib/cmake/SaxonCCoreHE" CACHE PATH "" FORCE)
find_package(SaxonCCoreHE 13.0.0 EXACT REQUIRED)
set_target_properties(SaxonCCoreHE::saxonc-core-he PROPERTIES
    MAP_IMPORTED_CONFIG_DEBUG Release
)

add_subdirectory(${_saxon_root}/src ${saxon_BINARY_DIR} EXCLUDE_FROM_ALL)

if(WIN32)
    # saxonc-he.dll is produced into CMAKE_RUNTIME_OUTPUT_DIRECTORY.
    # The imported core DLL is not; copy it to that same directory.
    get_target_property(_saxon_core_dll SaxonCCoreHE::saxonc-core-he IMPORTED_LOCATION_RELEASE)
    set(_saxon_core_dll_out "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/saxonc-core-he.dll")
    add_custom_command(
        OUTPUT "${_saxon_core_dll_out}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_saxon_core_dll}"
            "${_saxon_core_dll_out}"
        DEPENDS "${_saxon_core_dll}"
        VERBATIM
    )
    add_custom_target(saxonc_core_he_runtime ALL DEPENDS "${_saxon_core_dll_out}")
    add_dependencies(saxonc-he saxonc_core_he_runtime)
endif()

# leaf_spy is header-only and has its own repository.

FetchContent_Declare(
    leaf_spy
    GIT_REPOSITORY https://github.com/goloveshin/leaf_spy.git
    GIT_TAG main
)

# no tests or example of leaf_spy in this build
set(LEAF_SPY_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(leaf_spy)

# multi_index_lookup is header-only and has its own repository.

FetchContent_Declare(
    multi_index_lookup
    GIT_REPOSITORY https://github.com/goloveshin/multi_index_lookup.git
    GIT_TAG main
)

# no tests of multi_index_lookup in this build
set(MULTI_INDEX_LOOKUP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(multi_index_lookup)
