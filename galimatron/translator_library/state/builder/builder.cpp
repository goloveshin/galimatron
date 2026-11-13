// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"

// global
#include "global/common_library/utils.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state::builder
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_bld);
}


builder_t::builder_t(holder::language_part_t& state)
    : state_(state)
    , graph_(state_.graph())
{
    state_.set_root_node(create_root({}));
}

builder_t::builder_t(holder::language_part_t& state, on_existing_state_t)
    : state_(state)
    , graph_(state_.graph())
{
}

vertex_descriptor_t builder_t::add_vertex(const holder::vertex_t& vertex)
{
    const auto vd = boost::add_vertex(vertex, graph_);
    graph_[vd].vd_self = vd;
    if (vertex.vd_parent != vertex_descriptor_not_present)
    {
        boost::add_edge(vertex.vd_parent, vd, graph_);
    }
    return vd;
}

vertex_descriptor_t builder_t::clone_under(const clone_under_t& args)
{
    std::map<vertex_descriptor_t, index_t> string_sources;
    std::map<index_t, vertex_descriptor_t> string_targets;

    if (const auto* tag = std::get_if<tag_statement_t>(&graph_[args.source].entity))
    {
        for (const auto& [value_index, vd_string] : tag->string_values_at_zero_level)
        {
            string_sources.emplace(vd_string, value_index);
        }
    }

    return clone_under_impl({
        .source = args.source,
        .parent = args.parent,
        .tag_index = args.tag_index,
        .created_by_wizard = args.created_by_wizard,
        .string_sources = &string_sources,
        .string_targets = &string_targets
    });
}

vertex_descriptor_t builder_t::clone_under_impl(const clone_under_impl_t& args)
{
    auto copy = graph_[args.source];
    copy.vd_parent = args.parent;
    copy.vd_self = vertex_descriptor_not_present;
    if (auto* tag = std::get_if<tag_statement_t>(&copy.entity))
    {
        tag->string_values_at_zero_level.clear();
        if (args.tag_index)
        {
            tag->index_in_version = *args.tag_index;
        }
        tag->created_by_wizard = args.created_by_wizard;
    }

    const auto vd_new = add_vertex(copy);
    state_.note_version_child(vd_new);

    if (const auto found = args.string_sources->find(args.source); found != args.string_sources->end())
    {
        args.string_targets->emplace(found->second, vd_new);
    }

    std::vector<vertex_descriptor_t> children;
    for (const auto vd_child : subrange(boost::adjacent_vertices(args.source, graph_)))
    {
        children.push_back(vd_child);
    }
    for (const auto vd_child : children)
    {
        clone_under_impl({
            .source = vd_child,
            .parent = vd_new,
            .tag_index = {},
            .created_by_wizard = false,
            .string_sources = args.string_sources,
            .string_targets = args.string_targets
        });
    }

    if (std::get_if<tag_statement_t>(&graph_[args.source].entity))
    {
        std::get<tag_statement_t>(graph_[vd_new].entity).string_values_at_zero_level = *args.string_targets;
    }

    return vd_new;
}

vertex_descriptor_t builder_t::clone_from(const clone_from_t& args)
{
    std::map<vertex_descriptor_t, index_t> string_sources;
    std::map<index_t, vertex_descriptor_t> string_targets;

    if (const auto* tag = std::get_if<tag_statement_t>(&args.source_graph[args.source].entity))
    {
        for (const auto& [value_index, vd_string] : tag->string_values_at_zero_level)
        {
            string_sources.emplace(vd_string, value_index);
        }
    }

    return clone_from_impl({
        .source_graph = &args.source_graph,
        .source = args.source,
        .parent = args.parent,
        .tag_index = args.tag_index,
        .created_by_wizard = args.created_by_wizard,
        .clear_formatting = args.clear_formatting,
        .string_sources = &string_sources,
        .string_targets = &string_targets
    });
}

vertex_descriptor_t builder_t::clone_from_impl(const clone_from_impl_t& args)
{
    const auto& source_graph = *args.source_graph;
    auto copy = source_graph[args.source];
    copy.vd_parent = args.parent;
    copy.vd_self = vertex_descriptor_not_present;
    if (args.clear_formatting)
    {
        copy.user_formatting.reset();
        copy.user_comments.clear();
    }
    if (auto* tag = std::get_if<tag_statement_t>(&copy.entity))
    {
        tag->string_values_at_zero_level.clear();
        if (args.tag_index)
        {
            tag->index_in_version = *args.tag_index;
        }
        if (args.created_by_wizard)
        {
            tag->created_by_wizard = *args.created_by_wizard;
        }
    }

    const auto vd_new = add_vertex(copy);
    state_.note_version_child(vd_new);

    if (const auto found = args.string_sources->find(args.source); found != args.string_sources->end())
    {
        args.string_targets->emplace(found->second, vd_new);
    }

    std::vector<vertex_descriptor_t> children;
    for (const auto vd_child : subrange(boost::adjacent_vertices(args.source, source_graph)))
    {
        children.push_back(vd_child);
    }
    for (const auto vd_child : children)
    {
        clone_from_impl({
            .source_graph = args.source_graph,
            .source = vd_child,
            .parent = vd_new,
            .tag_index = {},
            .created_by_wizard = {},
            .clear_formatting = args.clear_formatting,
            .string_sources = args.string_sources,
            .string_targets = args.string_targets
        });
    }

    if (std::get_if<tag_statement_t>(&source_graph[args.source].entity))
    {
        std::get<tag_statement_t>(graph_[vd_new].entity).string_values_at_zero_level = *args.string_targets;
    }

    return vd_new;
}

vertex_descriptor_t builder_t::create_wizard_statement(const create_wizard_statement_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::WIZARD_STATEMENT,
        .entity = wizard_statement_t{
            .type = args.type,
            .context = args.context,
            .chunk_node = args.chunk_node,
            .chunk_uri = args.chunk_uri,
            .data = args.data,
            .commands = args.commands
        },
        .vd_parent = args.place_node
    });
    state_.note_wizard_statement(vd);
    return vd;
}

path_ot builder_t::chunk_path(const vertex_descriptor_t chunk_node) const
{
    const auto& chunk = graph_[chunk_node];
    if (!chunk.location || !chunk.location->path)
    {
        return {};
    }

    return chunk.location->path;
}

}// namespace galimatron::translator::state::builder
