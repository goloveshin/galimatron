#pragma once

// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/forward.hpp"


namespace galimatron::translator::state::holder
{

struct vertex_t
{
    entity_et entity_type;
    entity_t entity;
    location_ot location;
    vertex_descriptor_t vd_parent = vertex_descriptor_not_present;// not optional, because every vertex except one has it
    vertex_descriptor_t vd_self = vertex_descriptor_not_present;
    // // and /* */ to the left of this vertex; a version's keyword comment lives in the entity itself
    strings_t user_comments;
    // source text from the first token through the last, including spaces and line breaks
    // empty when the wizard created the vertex, or changed it after parsing
    content_ot user_formatting;
};
using vertex_rt = std::reference_wrapper<vertex_t>;
using vertex_crt = std::reference_wrapper<const vertex_t>;
using vertex_ort = std::optional<vertex_rt>;
using vertex_ocrt = std::optional<vertex_crt>;

}// namespace galimatron::translator::state::holder
