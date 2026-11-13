#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::gal2cpp
{

using namespace galimatron::translator::state;

struct translate_t
{
    deferred_file_contents_with_paths_t gal_state;

    // the number from gal.version; parser::parse selects it
    index_t gal_version;
};
holder::language_part_sptr_t translate(const translate_t&);

}// namespace galimatron::translator::transforms::gal2cpp
