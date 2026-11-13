#pragma once

// galimatron
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::history
{

// appends the <history> node under parent
//
void document(
    // the <galimatron> node
    pugi::xml_node parent,
    const state::holder::language_part_t& language,
    state::holder::compiled_part_t& compiled
);

}// namespace galimatron::translator::transforms::history
