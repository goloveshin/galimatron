#pragma once

namespace global::log_subsystems
{

// Logme subsystem ids. SID::Build takes const char*. every log.cfg subsystems.allowed lists these same ids

// brief: program entry
// part: galimatron/executable
//
inline constexpr const char* exec = "EXEC";

// brief: command line
// part: galimatron/executable_library
//
inline constexpr const char* exlib = "EXLIB";

// brief: when no narrower id fits
// part: galimatron/runtime_library
//
inline constexpr const char* rt = "RT";

// brief: reads and writes files
// part: galimatron/runtime_library/filesystem
//
inline constexpr const char* rt_fs = "RT-FS";

// brief: decorates the filesystem
// part: galimatron/runtime_library/filesystem_decorator
//
inline constexpr const char* rt_fsd = "RT-FSD";

// brief: reads gal.cfg and log.cfg
// part: galimatron/runtime_library/galimatron_config
//
inline constexpr const char* rt_cfg = "RT-CFG";

// brief: runs a translation
// part: galimatron/runtime_library/translate
//
inline constexpr const char* rt_trn = "RT-TRN";

// brief: writes the translation summary
// part: galimatron/runtime_library/translate_summary_xml
//
inline constexpr const char* rt_sum = "RT-SUM";

// brief: when no narrower id fits
// part: galimatron/translator_library
//
inline constexpr const char* tr = "TR";

// brief: parses GaL
// part: galimatron/translator_library/parser
//
inline constexpr const char* tr_par = "TR-PAR";

// brief: builds the language graph
// part: galimatron/translator_library/state/builder
//
inline constexpr const char* tr_bld = "TR-BLD";

// brief: from the language cpp presentation to the post-language cpp presentation
// part: galimatron/translator_library/transforms/cpp2post_cpp
//
inline constexpr const char* tr_cmp = "TR-CMP";

// brief: from the language gal presentation to the language cpp presentation
// part: galimatron/translator_library/transforms/gal2cpp
//
inline constexpr const char* tr_g2s = "TR-G2S";

// brief: holds the language graph and the compiled graph
// part: galimatron/translator_library/state/holder
//
inline constexpr const char* tr_hld = "TR-HLD";

// brief: from the language cpp presentation to the language gal presentation
// part: galimatron/translator_library/transforms/cpp2gal
//
inline constexpr const char* tr_s2g = "TR-S2G";

// brief: from the language cpp presentation to the language cpp presentation; applies the wizard command
// part: galimatron/translator_library/transforms/cpp2cpp_wizard
//
inline constexpr const char* tr_s2s = "TR-S2S";

// brief: from the language cpp presentation to the language xml presentation
// part: galimatron/translator_library/transforms/cpp2xml
//
inline constexpr const char* tr_s2x = "TR-S2X";

// brief: from the language xml presentation to the language cpp presentation
// part: galimatron/translator_library/transforms/xml2cpp
//
inline constexpr const char* tr_x2s = "TR-X2S";

// brief: from an xml presentation to an xml presentation; runs the user xslt
// part: galimatron/translator_library/transforms/xml2xml
//
inline constexpr const char* tr_xslt = "TR-XSLT";

// brief: cpp2cpp_validate checks the language cpp presentation and leaves that presentation. wrappers in galimatron/translator_library/translator/detail/transforms stay on this id
// part: galimatron/translator_library/translator, galimatron/translator_library/transforms/cpp2cpp_validate
//
inline constexpr const char* tr_tsp = "TR-TSP";

// brief: wizard commands
// part: galimatron/translator_library/state/wizard
//
inline constexpr const char* tr_wiz = "TR-WIZ";

}// namespace global::log_subsystems
