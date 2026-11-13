#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;

holder::compiled_part_sptr_t translate(const holder::language_part_sptr_t);

}// namespace galimatron::translator::transforms::cpp2post_cpp
