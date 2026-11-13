#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"


namespace galimatron::translator::state
{

using vertex_descriptor_t = std::size_t;
using vertex_descriptor_ot = std::optional<vertex_descriptor_t>;
using vertex_descriptors_t = std::deque<vertex_descriptor_t>;
using vertex_descriptors_ot = std::optional<vertex_descriptors_t>;

constexpr auto vertex_descriptor_not_present = std::numeric_limits<vertex_descriptor_t>::max();

}// namespace galimatron::translator::state
