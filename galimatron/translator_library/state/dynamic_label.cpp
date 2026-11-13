// galimatron
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// global
#include "global/common_library/utils.hpp"


namespace galimatron::translator::state
{

namespace
{

bool is_version_definition(const holder::vertex_t& vertex)
{
    return std::holds_alternative<root_aggregate_version_definition_t>(vertex.entity)
        || std::holds_alternative<aggregate_version_definition_t>(vertex.entity);
}

index_ot version_index_of(const holder::vertex_t& vertex)
{
    if (const auto* definition = std::get_if<root_aggregate_version_definition_t>(&vertex.entity))
    {
        if (definition->index)
        {
            return definition->index;
        }
        return definition->owner;
    }
    if (const auto* definition = std::get_if<aggregate_version_definition_t>(&vertex.entity))
    {
        if (definition->index)
        {
            return definition->index;
        }
        return definition->owner;
    }
    return {};
}

vertex_descriptor_ot version_definition_of(
    const holder::language_part_t::graph_t& graph,
    vertex_descriptor_t vd
)
{
    while (vd != vertex_descriptor_not_present)
    {
        if (is_version_definition(graph[vd]))
        {
            return vd;
        }
        vd = graph[vd].vd_parent;
    }
    return {};
}

vertex_descriptor_t aggregate_of_definition(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptor_t vd_definition
)
{
    const auto vd_definitions = graph[vd_definition].vd_parent;
    return graph[vd_definitions].vd_parent;
}

// the label that wins is the latest one still visible to the tag.
// root labels of a version are visible before that version's own labels
//
struct seen_label_t
{
    index_t version_index = 0;
    index_t index_in_version = 0;
    bool from_root = false;
    vertex_descriptor_t vd = vertex_descriptor_not_present;
};
bool is_later(const seen_label_t& candidate, const seen_label_t& current)
{
    if (candidate.version_index != current.version_index)
    {
        return candidate.version_index > current.version_index;
    }
    if (candidate.from_root != current.from_root)
    {
        return !candidate.from_root;
    }
    return candidate.index_in_version > current.index_in_version;
}

void consider_definition(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptor_t vd_definition,
    const index_t version_index,
    const bool from_root,
    const index_ot up_to_index,
    const name_t& label_name,
    std::optional<seen_label_t>& best,
    // numbered statements sort after anonymous statements of the same version
    const index_t index_base = 0
)
{
    for (const auto vd_child : subrange(boost::adjacent_vertices(vd_definition, graph)))
    {
        const auto* label = std::get_if<label_statement_t>(&graph[vd_child].entity);
        if (!label || label->name != label_name)
        {
            continue;
        }
        if (up_to_index && label->index_in_version >= *up_to_index)
        {
            continue;
        }

        const seen_label_t candidate{
            .version_index = version_index,
            .index_in_version = label->index_in_version + index_base,
            .from_root = from_root,
            .vd = vd_child
        };
        if (!best || is_later(candidate, *best))
        {
            best = candidate;
        }
    }
}

}// namespace

string_ot dynamic_value_of_label(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_tag,
    const name_t& label_name
)
{
    if (label_name == gal_and_xml_tags::label::important)
    {
        return {};
    }

    const auto& graph = state.graph();
    const auto vd_definition = version_definition_of(graph, vd_tag);
    if (!vd_definition)
    {
        return {};
    }

    const auto version_index = version_index_of(graph[*vd_definition]);
    if (!version_index)
    {
        return {};
    }

    const auto& tag = std::get<tag_statement_t>(graph[vd_tag].entity);
    const auto vd_aggregate = aggregate_of_definition(graph, *vd_definition);
    const auto vd_root = state.aggregates().root().vd;
    const bool tag_is_in_root = vd_aggregate == vd_root;

    std::optional<seen_label_t> best;

    auto consider_aggregate = [&](
        const vertex_descriptor_t vd_aggregate_node,
        const bool from_root,
        const bool limit_current
    )
    {
        // a numbered body's labels sort after the anonymous bodies it owns
        const index_t numbered_base = std::numeric_limits<index_t>::max() / 2;
        const bool tag_in_this_aggregate = aggregate_of_definition(graph, *vd_definition) == vd_aggregate_node;

        for (const auto& definition : state.versions().definitions_in_order(vd_aggregate_node))
        {
            if (definition.index > *version_index)
            {
                continue;
            }

            const auto anonymous = state.versions().anonymous_owned_by(vd_aggregate_node, definition.index);
            index_ot tag_anonymous_order;
            if (tag_in_this_aggregate)
            {
                for (const auto& body : anonymous)
                {
                    if (body.vd_definition == *vd_definition)
                    {
                        tag_anonymous_order = body.definition_order;
                    }
                }
            }

            for (const auto& body : anonymous)
            {
                index_ot up_to;
                if (limit_current && definition.index == *version_index && tag_anonymous_order)
                {
                    if (body.definition_order == *tag_anonymous_order)
                    {
                        up_to = tag.index_in_version;
                    }
                    else if (body.definition_order > *tag_anonymous_order)
                    {
                        up_to = index_t{ 0 };
                    }
                }
                consider_definition(
                    graph,
                    body.vd_definition,
                    definition.index,
                    from_root,
                    up_to,
                    label_name,
                    best
                );
            }

            index_ot up_to;
            if (limit_current && definition.index == *version_index)
            {
                if (tag_anonymous_order)
                {
                    up_to = index_t{ 0 };
                }
                else if (tag_in_this_aggregate)
                {
                    up_to = tag.index_in_version;
                }
            }
            consider_definition(
                graph,
                definition.vd,
                definition.index,
                from_root,
                up_to,
                label_name,
                best,
                numbered_base
            );
        }
    };

    consider_aggregate(vd_root, true, tag_is_in_root);
    if (!tag_is_in_root)
    {
        consider_aggregate(vd_aggregate, false, true);
    }

    if (!best)
    {
        return {};
    }

    for (const auto vd_child : subrange(boost::adjacent_vertices(best->vd, graph)))
    {
        if (const auto* dynamic = std::get_if<label_dynamic_t>(&graph[vd_child].entity))
        {
            return dynamic->value;
        }
    }
    return {};
}

strings_t dynamic_values_of_tag(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_tag
)
{
    strings_t values;
    const auto& tag = std::get<tag_statement_t>(state.graph()[vd_tag].entity);
    for (const auto& label : tag.labels)
    {
        if (const auto value = dynamic_value_of_label(state, vd_tag, label))
        {
            values.push_back(*value);
        }
    }
    return values;
}

bool tag_uses_dynamic_label(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_tag
)
{
    return !dynamic_values_of_tag(state, vd_tag).empty();
}

}// namespace galimatron::translator::state
