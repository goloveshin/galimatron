cmake_minimum_required(VERSION 4.0)

antlr4_generate(galimatron_lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/g_lexer.g4
    LEXER
    FALSE # not used for LEXER
    FALSE # not used for LEXER
    "galimatron" # namespace
)

antlr4_generate(galimatron_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/g_parser.g4
    PARSER
    TRUE # listener
    FALSE # visitor
    "galimatron" # namespace
    "${ANTLR4_TOKEN_FILES_galimatron_lexer}"
    "${ANTLR4_TOKEN_DIRECTORY_galimatron_lexer}"
)

add_library(
    galimatron_translator_library
    STATIC
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.hpp *.cpp *.g4)

target_sources(
    galimatron_translator_library
    PRIVATE
        ${SOURCES}
        ${ANTLR4_SRC_FILES_galimatron_lexer}
        ${ANTLR4_SRC_FILES_galimatron_parser}
)

target_include_directories(
    galimatron_translator_library
    PUBLIC
        ${ANTLR4_INCLUDE_DIR_galimatron_lexer}
        ${ANTLR4_INCLUDE_DIR_galimatron_parser}
)

target_link_libraries(
    galimatron_translator_library
    PUBLIC
        ${Boost_LIBRARIES}
        Boost::hash2
        range-v3::range-v3
        pugixml::pugixml
        antlr4_static
        galimatron_common_library
        leaf_spy::leaf_spy
    PRIVATE
        Boost::dll_std_fs
        SaxonCHE::saxonc-he
        SaxonCCoreHE::saxonc-core-he
        Taskflow::Taskflow
        pantor::inja
)

target_precompile_headers(
    galimatron_translator_library
    REUSE_FROM
        global_pch_library
)

# the tag-name regex reads identifier_pattern from the generated lexer header
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/transforms/cpp2post_cpp/detail/naming.cpp
    PROPERTIES
        OBJECT_DEPENDS "${ANTLR4_INCLUDE_DIR_galimatron_lexer}/g_lexer.h"
)
