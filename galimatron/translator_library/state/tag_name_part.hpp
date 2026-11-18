#pragma once

// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/holder/graph/graph.hpp"


namespace galimatron::translator::state
{

// where the tag-name-from of a static label points inside a tag
//
struct tag_name_part_t
{
    enum class status_et
    {
        // the part is not one of the label's values
        NOT_AMONG_VALUES,
        // the tag has no string at the position of that part
        NO_VALUE,
        FOUND
    };

    status_et status = status_et::NOT_AMONG_VALUES;
    // the position of the part among the values of the label
    index_t index = 0;
    // the string of the tag at that position. set when FOUND
    const std::string* value = nullptr;
};

// the label must have tag-name-from.
// the caller decides what a missing part means: the compiler refuses the tag,
// a collapse leaves the tag without a name
//
// testing: yes (unit)
//
tag_name_part_t tag_name_part(
    const holder::vertex_graph::graph_t& graph,
    const label_static_t& label,
    const tag_statement_t& tag
);

}// namespace galimatron::translator::state
