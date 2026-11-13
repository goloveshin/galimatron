#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::cpp2gal
{

using namespace galimatron::translator::state;

struct translate_t
{
    holder::language_part_sptr_t language_state;
};
file_contents_with_paths_t translate(const translate_t&);

}// namespace galimatron::translator::transforms::cpp2gal
