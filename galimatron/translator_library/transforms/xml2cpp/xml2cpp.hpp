#pragma once

// galimatron
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/transforms/layout_table.hpp"


namespace galimatron::translator::transforms::xml2cpp
{

using namespace galimatron::translator::state;

// a language document that never entered Saxon
//
struct translate_t
{
    pugi_document_with_meanful_node_t document;

    // the table cpp2xml filled for this document; null leaves every layout empty
    layout::table_t* layout_table = nullptr;
};

// unused. kept so a later processor can read a document that never entered Saxon
//
// testing: no
//
holder::language_part_sptr_t translate(const translate_t&);

}// namespace galimatron::translator::transforms::xml2cpp
