#pragma once

// galimatron
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::history::detail
{

using namespace galimatron::translator::state;

// appends the <history> node under parent
//
void build(
    // the <galimatron> node
    pugi::xml_node parent,
    const holder::language_part_t& language,
    holder::compiled_part_t& compiled
);

}// namespace galimatron::translator::transforms::history::detail
