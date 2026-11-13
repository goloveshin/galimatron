#pragma once

// galimatron
#include "galimatron/translator_library/parser/fill_state_from_content.hpp"


namespace galimatron::translator::parser
{

// Implementations of the parsing (parse_content_ft) and walk (walk_ft) functions
// on the GaL grammar for fill_state_from_content().
//
single_parse_state_at parse(const file_content_t&);

// parser for the selected GaL version. currently this is only gal_version::current,
// otherwise exception::subdomain::translator_library::parser
//
// testing: yes (unit)
//
single_parse_state_at parse(index_t gal_version, const file_content_t&);

void walk(single_parse_state_at&, antlr_listener_t*);

}// namespace galimatron::translator::parser
