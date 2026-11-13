#pragma once

// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/cpp2post_cpp_impl.hpp"


namespace galimatron::translator::transforms::cpp2post_cpp
{

[[noreturn]] void throw_compiler(const std::string& what);

// chunk file in which the vertex is declared
//
inline path_ot file_path_of_tag(const auto& graph, vertex_descriptor_t vd)
{
    while (vd != vertex_descriptor_not_present)
    {
        const auto& vertex = graph[vd];
        if (vertex.entity_type == entity_et::CHUNK)
        {
            return vertex.location ? vertex.location->path : path_ot{};
        }
        vd = vertex.vd_parent;
    }

    return {};
}

// once and local live on the version definition. an absent mode inherits
//
inline tag_mode_et version_mode_of(const auto& vertex)
{
    if (const auto* definition = std::get_if<root_aggregate_version_definition_t>(&vertex.entity))
    {
        return definition->mode;
    }
    if (const auto* definition = std::get_if<aggregate_version_definition_t>(&vertex.entity))
    {
        return definition->mode;
    }
    return tag_mode_et::INHERIT;
}

// the vertex is declared inside this aggregate
//
inline bool entity_belongs_to_aggregate(const auto& graph, vertex_descriptor_t vd, const vertex_descriptor_t vd_aggregate)
{
    while (vd != vertex_descriptor_not_present)
    {
        if (vd == vd_aggregate)
        {
            return true;
        }
        vd = graph[vd].vd_parent;
    }

    return false;
}

// once keeps statements that came from the root.
// local keeps this version only.
// own keeps earlier statements of this aggregate and drops the root
//
inline bool statement_reaches_version(
    const auto& graph,
    const vertex_descriptor_t vd_aggregate,
    const tag_mode_et version_mode,
    const index_t compiled_version,
    const index_t statement_version,
    const vertex_descriptor_t vd_statement
)
{
    if (statement_version == compiled_version)
    {
        return true;
    }
    if (statement_version > compiled_version)
    {
        return false;
    }
    if (version_mode == tag_mode_et::INHERIT)
    {
        return true;
    }
    if (version_mode == tag_mode_et::LOCAL)
    {
        return false;
    }
    if (version_mode == tag_mode_et::OWN)
    {
        return entity_belongs_to_aggregate(graph, vd_statement, vd_aggregate);
    }
    return !entity_belongs_to_aggregate(graph, vd_statement, vd_aggregate);
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
