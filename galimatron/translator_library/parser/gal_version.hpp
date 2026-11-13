#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace global::filesystem::abstract
{
struct filesystem_t;
}

namespace galimatron::translator::parser::gal_version
{

// file name in the incoming .gal directory
inline constexpr std::string_view file_name = "gal.version";

// the parser that exists now
inline constexpr index_t current = 1;

// version number from the gal.version text.
// empty text, zero, and extra characters — exception::subdomain::translator_library::parser
//
// testing: yes (unit)
//
index_t from_content(const content_t&);

// no file in input_gal_dir — current, otherwise from_content
//
// testing: yes (unit)
//
index_t from_input_dir(
    const gf::abstract::filesystem_t&,
    const path_t& input_gal_dir
);

// version is not current — exception::subdomain::translator_library::parser
//
// testing: yes (unit)
//
void require_supported(index_t gal_version);

}// namespace galimatron::translator::parser::gal_version
