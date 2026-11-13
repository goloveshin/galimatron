#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::translator::translator::builtin_transform_xml
{

// state names in intermediate-xml-dir.
// lang-parsed.xml is written by validation, after the parsed graph compiles.
// the other constants name the transform that writes the file.
// each description is the description attribute of <galimatron> node in that file
inline constexpr std::string_view gal2cpp = "lang-parsed.xml";
inline constexpr std::string_view gal2cpp_description = "after parsing, before the wizard";
inline constexpr std::string_view cpp2xml = "lang-wizard.xml";
inline constexpr std::string_view cpp2xml_description = "after the wizard";
inline constexpr std::string_view xml2cpp = "lang-xslt.xml";
inline constexpr std::string_view xml2cpp_description = "after the user xslt, before the seal";
inline constexpr std::string_view cpp2gal = "lang-final.xml";
inline constexpr std::string_view cpp2post_xml = "post-compiled.xml";
// lang-final.xml and post-compiled.xml
inline constexpr std::string_view sealed_description = "after the seal";

}// namespace galimatron::translator::translator::builtin_transform_xml
