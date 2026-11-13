#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/transforms/layout_table.hpp"
#include "galimatron/translator_library/transforms/xml2xml/xml2xml.hpp"


namespace galimatron::translator::transforms::xml2cpp_saxon
{

using namespace galimatron::translator::state;

// walks the Saxon document. the session stays alive for the walk.
// the pugixml xml2cpp stays for a document that never entered Saxon
//
// testing: no
//
holder::language_part_sptr_t translate(
    xml2xml::session_state_t& session,
    layout::table_t* layout_table
);

}// namespace galimatron::translator::transforms::xml2cpp_saxon
