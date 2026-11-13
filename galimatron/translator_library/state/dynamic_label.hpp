#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::state
{

// dynamic id of a label visible to this tag; empty when the label is static or builtin
//
string_ot dynamic_value_of_label(
    const holder::language_part_t& state,
    vertex_descriptor_t vd_tag,
    const name_t& label_name
);

// dynamic ids of the tag's labels, in label order
//
strings_t dynamic_values_of_tag(
    const holder::language_part_t& state,
    vertex_descriptor_t vd_tag
);

// the tag references at least one dynamic label
//
bool tag_uses_dynamic_label(
    const holder::language_part_t& state,
    vertex_descriptor_t vd_tag
);

}// namespace galimatron::translator::state
