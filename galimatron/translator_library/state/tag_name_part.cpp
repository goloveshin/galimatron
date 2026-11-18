// galimatron
#include "galimatron/translator_library/state/tag_name_part.hpp"


namespace galimatron::translator::state
{

tag_name_part_t tag_name_part(
    const holder::vertex_graph::graph_t& graph,
    const label_static_t& label,
    const tag_statement_t& tag
)
{
    const auto part = ranges::find(label.values, label.tag_name_from.value());
    if (part == ranges::end(label.values))
    {
        return {
            .status = tag_name_part_t::status_et::NOT_AMONG_VALUES
        };
    }

    const auto index = static_cast<index_t>(ranges::distance(ranges::begin(label.values), part));
    const auto value = tag.string_values_at_zero_level.find(index);
    if (value == tag.string_values_at_zero_level.end())
    {
        return {
            .status = tag_name_part_t::status_et::NO_VALUE,
            .index = index
        };
    }

    return {
        .status = tag_name_part_t::status_et::FOUND,
        .index = index,
        .value = &get_entity<tag_string_t>(graph[value->second]).value
    };
}

}// namespace galimatron::translator::state
