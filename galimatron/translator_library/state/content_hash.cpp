// galimatron
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// Boost
#include <boost/hash2/sha2.hpp>


namespace galimatron::translator::content_hash
{

namespace
{

void append_counted(std::string& out, const char kind, const std::string_view text)
{
    out += kind;
    out += std::to_string(text.size());
    out += ':';
    out += text;
    out += ';';
}

void append_value(
    std::string& out,
    const state::holder::vertex_graph::graph_t& graph,
    const state::vertex_descriptor_t vd
)
{
    const auto& vertex = graph[vd];

    if (const auto* string = std::get_if<state::tag_string_t>(&vertex.entity))
    {
        append_counted(out, 's', string->value);
        return;
    }

    if (const auto* keyvalue = std::get_if<state::tag_keyvalue_t>(&vertex.entity))
    {
        append_counted(out, 'k', keyvalue->key.value_or(""));
        const auto [begin, end] = boost::adjacent_vertices(vd, graph);
        for (auto it = begin; it != end; ++it)
        {
            append_value(out, graph, *it);
        }
        return;
    }

    if (std::holds_alternative<state::tag_array_t>(vertex.entity))
    {
        out += 'a';
    }
    else if (std::holds_alternative<state::tag_record_t>(vertex.entity)
        || std::holds_alternative<state::tag_value_t>(vertex.entity)
        || std::holds_alternative<state::tag_values_t>(vertex.entity))
    {
        out += 'r';
    }

    const auto [begin, end] = boost::adjacent_vertices(vd, graph);
    for (auto it = begin; it != end; ++it)
    {
        append_value(out, graph, *it);
    }
    out += ';';
}

void append_tag(
    std::string& out,
    const state::holder::vertex_graph::graph_t& graph,
    const state::vertex_descriptor_t vd
)
{
    const auto [begin, end] = boost::adjacent_vertices(vd, graph);
    for (auto it = begin; it != end; ++it)
    {
        if (graph[*it].entity_type == state::entity_et::TAG_VALUES)
        {
            append_value(out, graph, *it);
        }
    }
}

bool is_important(const state::tag_statement_t& tag)
{
    return std::ranges::find(tag.labels, std::string{ state::gal_and_xml_tags::label::important }) != tag.labels.end();
}

}// namespace

std::string sha256_hex(const std::string_view bytes)
{
    boost::hash2::sha2_256 hash;
    hash.update(bytes.data(), bytes.size());
    return boost::hash2::to_string(hash.result());
}

std::string important_tags_hash(
    const state::holder::vertex_graph::graph_t& graph,
    const state::vertex_descriptor_ot vd_definition
)
{
    std::string text;
    if (vd_definition)
    {
        const auto [begin, end] = boost::adjacent_vertices(*vd_definition, graph);
        for (auto it = begin; it != end; ++it)
        {
            if (graph[*it].entity_type != state::entity_et::TAG_STATEMENT)
            {
                continue;
            }

            const auto& tag = std::get<state::tag_statement_t>(graph[*it].entity);
            if (!is_important(tag))
            {
                continue;
            }
            append_tag(text, graph, *it);
        }
    }
    return sha256_hex(text);
}

std::string hash_of_hashes(const std::vector<std::string>& hashes)
{
    auto ordered = hashes;
    ordered |= ranges::actions::sort;

    std::string text;
    for (const auto& hash : ordered)
    {
        append_counted(text, 'h', hash);
    }
    return sha256_hex(text);
}

}// namespace galimatron::translator::content_hash
