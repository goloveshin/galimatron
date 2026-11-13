#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::translator::translator::builtin_transform_name
{

// names of the builtin transforms
inline constexpr std::string_view gal2cpp = "gal2cpp";
inline constexpr std::string_view cpp2cpp_validate = "cpp2cpp_validate";
inline constexpr std::string_view cpp2cpp_wizard = "cpp2cpp_wizard";
inline constexpr std::string_view cpp2cpp_before_xslt = "cpp2cpp_before_xslt";
inline constexpr std::string_view cpp2cpp_after_xslt = "cpp2cpp_after_xslt";
inline constexpr std::string_view cpp2xml = "cpp2xml";
inline constexpr std::string_view xml2saxon = "xml2saxon";
inline constexpr std::string_view xml2cpp = "xml2cpp";
inline constexpr std::string_view cpp2gal = "cpp2gal";
inline constexpr std::string_view cpp2post_cpp = "cpp2post_cpp";
inline constexpr std::string_view post_cpp2post_xml = "post_cpp2post_xml";
inline constexpr std::string_view post_xml2saxon = "post_xml2saxon";

}// namespace galimatron::translator::translator::builtin_transform_name
