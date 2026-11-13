#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::cpp2post_cpp::template_engine
{

using namespace galimatron::translator::state;

// one rendered _te string. the language value stays as written
//
struct rendered_string_t
{
    vertex_descriptor_t vd_string;
    std::string value;
};

// renders the _te strings of these tags
// the first tag of a name wins: compilation passes the latest declaration first, so an inherited tag uses this version's values
// one tag value is that value. several values are an array: {{name}} is the first, {{name.2}} is index 2
// a record with keys is a JSON object, so {{pack.box}} reads a field
// a record with only keyless values is a JSON array
// keyless entries in a keyed record are not template data
// throws template_engine when inja rejects the template
//
std::deque<rendered_string_t> render(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptors_t& tags
);

}// namespace galimatron::translator::transforms::cpp2post_cpp::template_engine
