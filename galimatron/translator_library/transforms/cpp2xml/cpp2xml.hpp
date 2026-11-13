#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/transforms/layout_table.hpp"


namespace galimatron::translator::transforms::cpp2xml
{

using namespace galimatron::translator::state;

struct translate_t
{
    const holder::language_part_sptr_t language_state;
    const holder::compiled_part_sptr_ot compiled_state;

    gf::split_path_by_parts_ft split_path_by_parts;

    // filled while the document is built; null leaves the document unmarked
    layout::table_t* layout_table = nullptr;

    // the <galimatron> node description attribute. empty leaves <galimatron> node without the attribute
    const std::string_view description = {};
};
pugi_document_with_meanful_node_t translate(const translate_t&);

}// namespace galimatron::translator::transforms::cpp2xml
