#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

// applies the single wizard command and returns that language state.
// no command leaves the state unchanged
//
holder::language_part_sptr_t translate(const holder::language_part_sptr_t);

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
