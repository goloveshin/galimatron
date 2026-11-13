// galimatron
#include "galimatron/translator_library/state/holder/graph/vertex_payload.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"

// global
#include "global/common_library/utils.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state::holder::vertex_payload
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_hld);
}


void append_vertex(std::string& out, const vertex_graph::graph_t& graph, const vertex_descriptor_t vd)
{
    const auto& vertex = graph[vd];
    out.append(to_string(vertex.entity_type));
    out.push_back('{');

    if (const auto* tag = std::get_if<tag_statement_t>(&vertex.entity))
    {
        out.append(std::to_string(tag->index_in_version));
        if (tag->name)
        {
            out.push_back(' ');
            out.append(*tag->name);
        }
        for (const auto& label : tag->labels)
        {
            out.push_back('|');
            out.append(label);
        }
    }
    else if (const auto* text = std::get_if<tag_string_t>(&vertex.entity))
    {
        out.append(text->value);
    }
    else if (const auto* key = std::get_if<tag_keyvalue_t>(&vertex.entity))
    {
        if (key->key)
        {
            out.append(*key->key);
        }
    }
    else if (const auto* label = std::get_if<label_statement_t>(&vertex.entity))
    {
        out.append(std::to_string(label->index_in_version));
        out.push_back(' ');
        out.append(label->name);
    }
    else if (const auto* label_static = std::get_if<label_static_t>(&vertex.entity))
    {
        for (const auto& value : label_static->values)
        {
            out.push_back('|');
            out.append(value);
        }
        if (label_static->tag_name_from)
        {
            out.append(*label_static->tag_name_from);
        }
    }
    else if (const auto* label_dynamic = std::get_if<label_dynamic_t>(&vertex.entity))
    {
        out.append(label_dynamic->value);
    }
    else if (const auto* ordering = std::get_if<ordering_statement_t>(&vertex.entity))
    {
        out.append(std::to_string(ordering->index_in_version));
        out.push_back(' ');
        out.append(ordering->name);
        for (const auto& part : ordering->ordering_parts)
        {
            for (const auto& aggregate_name : part)
            {
                out.push_back('|');
                out.append(aggregate_name);
            }
        }
    }
    else if (const auto* operation = std::get_if<operation_statement_t>(&vertex.entity))
    {
        out.append(std::to_string(operation->index_in_version));
        out.push_back(' ');
        out.append(operation->name);
        if (operation->is_version)
        {
            out.append("#version");
        }
    }

    std::vector<vertex_descriptor_t> child_vertices;
    for (const auto vd_child : ::subrange(boost::adjacent_vertices(vd, graph)))
    {
        child_vertices.push_back(vd_child);
    }
    for (const auto vd_child : child_vertices)
    {
        append_vertex(out, graph, vd_child);
    }
    out.push_back('}');
}

std::string vertex(const vertex_graph::graph_t& graph, const vertex_descriptor_t vd)
{
    std::string text;
    append_vertex(text, graph, vd);
    return text;
}

std::string children(const vertex_graph::graph_t& graph, const vertex_descriptor_ot vd)
{
    if (!vd)
    {
        return {};
    }

    std::string text;
    std::vector<vertex_descriptor_t> child_vertices;
    for (const auto vd_child : ::subrange(boost::adjacent_vertices(*vd, graph)))
    {
        child_vertices.push_back(vd_child);
    }
    for (const auto vd_child : child_vertices)
    {
        append_vertex(text, graph, vd_child);
    }
    return text;
}

}// namespace galimatron::translator::state::holder::vertex_payload
