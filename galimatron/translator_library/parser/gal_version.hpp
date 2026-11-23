#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace virtual_filesystem
{
struct filesystem_t;
}// namespace virtual_filesystem

namespace galimatron::translator::parser::gal_version
{

// a function here that throws uses exception::subdomain::translator_library::parser

// file name in the incoming .gal directory
inline constexpr std::string_view file_name = "gal.version";

// the parser that exists now
inline constexpr index_t current = 1;

// version number from the gal.version text.
// throws: empty text, zero, or extra characters
//
// testing: yes (unit)
//
index_t from_content(const content_t&);

// no file in input_gal_dir gives current.
// otherwise the version comes from from_content
//
// testing: yes (unit)
//
index_t from_input_dir(
    const virtual_filesystem::filesystem_t&,
    const path_t& input_gal_dir
);

// requires the version to be current.
// throws: the version is not current
//
// testing: yes (unit)
//
void require_supported(index_t gal_version);

}// namespace galimatron::translator::parser::gal_version
