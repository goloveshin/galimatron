// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/template_engine.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/utils.hpp"

// inja
#include <inja/inja.hpp>


namespace galimatron::translator::transforms::cpp2post_cpp::template_engine
{

using namespace galimatron::translator::state;

namespace
{

// throws template_engine
//
[[noreturn]] void fail(std::string what)
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::template_engine{},
        exception::e_what_info{ std::move(what) }
    );
}

nlohmann::json json_of(const holder::language_part_t::graph_t& graph, const vertex_descriptor_t vd)
{
    const auto& vertex = graph[vd];

    if (vertex.entity_type == entity_et::TAG_STRING)
    {
        return get_entity<tag_string_t>(vertex).value;
    }

    if (vertex.entity_type == entity_et::TAG_VALUE)
    {
        nlohmann::json result;
        for (const auto vd_child : subrange(boost::adjacent_vertices(vd, graph)))
        {
            result = json_of(graph, vd_child);
        }
        return result;
    }

    if (vertex.entity_type == entity_et::TAG_ARRAY)
    {
        auto array = nlohmann::json::array();
        for (const auto vd_child : subrange(boost::adjacent_vertices(vd, graph)))
        {
            array.push_back(json_of(graph, vd_child));
        }
        return array;
    }

    if (vertex.entity_type == entity_et::TAG_RECORD)
    {
        auto object = nlohmann::json::object();
        auto array = nlohmann::json::array();
        auto keyed = false;
        for (const auto vd_child : subrange(boost::adjacent_vertices(vd, graph)))
        {
            if (graph[vd_child].entity_type != entity_et::TAG_KEYVALUE)
            {
                continue;
            }
            const auto& keyvalue = get_entity<tag_keyvalue_t>(graph[vd_child]);
            nlohmann::json payload;
            for (const auto vd_value : subrange(boost::adjacent_vertices(vd_child, graph)))
            {
                payload = json_of(graph, vd_value);
            }
            if (keyvalue.key)
            {
                object[*keyvalue.key] = std::move(payload);
                keyed = true;
            }
            else
            {
                array.push_back(std::move(payload));
            }
        }
        if (!keyed)
        {
            return array;
        }
        return object;
    }

    return {};
}

struct version_data_t
{
    nlohmann::json data = nlohmann::json::object();
    // tags with more than one value. {{name}} means the first of these
    strings_t several;
};

// the first tag of a name wins
//
version_data_t version_data(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptors_t& tags
)
{
    version_data_t result;
    for (const auto vd_statement : tags)
    {
        if (graph[vd_statement].entity_type != entity_et::TAG_STATEMENT)
        {
            continue;
        }
        const auto& statement = get_entity<tag_statement_t>(graph[vd_statement]);
        if (!statement.name || result.data.contains(*statement.name))
        {
            continue;
        }

        auto values = nlohmann::json::array();
        for (const auto vd_child : subrange(boost::adjacent_vertices(vd_statement, graph)))
        {
            if (graph[vd_child].entity_type != entity_et::TAG_VALUES)
            {
                continue;
            }
            for (const auto vd_value : subrange(boost::adjacent_vertices(vd_child, graph)))
            {
                values.push_back(json_of(graph, vd_value));
            }
        }

        if (values.empty())
        {
            continue;
        }
        if (values.size() == 1)
        {
            result.data[*statement.name] = values.front();
        }
        else
        {
            result.data[*statement.name] = values;
            result.several.push_back(*statement.name);
        }
    }
    return result;
}

void collect_templates(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptor_t vd,
    vertex_descriptors_t& templates
)
{
    if (graph[vd].entity_type == entity_et::TAG_STRING)
    {
        const auto& text = get_entity<tag_string_t>(graph[vd]);
        if (text.is_template)
        {
            templates.push_back(vd);
        }
    }

    for (const auto vd_child : subrange(boost::adjacent_vertices(vd, graph)))
    {
        collect_templates(graph, vd_child, templates);
    }
}

// {{name}} is the first value when the tag holds several. {{name.2}} stays an index
//
std::string first_value_syntax(std::string text, const strings_t& several)
{
    for (const auto& name : several)
    {
        const auto bare = std::string("{{") + name + "}}";
        const auto bare_spaced = std::string("{{ ") + name + " }}";
        boost::algorithm::replace_all(text, bare, std::string("{{") + name + ".0}}");
        boost::algorithm::replace_all(text, bare_spaced, std::string("{{ ") + name + ".0 }}");
    }
    return text;
}

}// namespace

std::deque<rendered_string_t> render(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptors_t& tags
)
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::template_engine{}
    );

    vertex_descriptors_t templates;
    for (const auto vd_tag : tags)
    {
        collect_templates(graph, vd_tag, templates);
    }
    if (templates.empty())
    {
        return {};
    }

    const auto data = version_data(graph, tags);
    inja::Environment environment;
    std::deque<rendered_string_t> rendered;
    for (const auto vd_string : templates)
    {
        const auto& text = get_entity<tag_string_t>(graph[vd_string]);
        const auto prepared = first_value_syntax(text.value, data.several);
        try
        {
            rendered.push_back({
                .vd_string = vd_string,
                .value = environment.render(prepared, data.data)
            });
        }
        catch (const inja::InjaError& error)
        {
            fail(std::format("template '{}' failed: {}", text.value, error.what()));
        }
        catch (const nlohmann::json::exception& error)
        {
            fail(std::format("template '{}' failed: {}", text.value, error.what()));
        }
    }
    return rendered;
}

}// namespace galimatron::translator::transforms::cpp2post_cpp::template_engine
