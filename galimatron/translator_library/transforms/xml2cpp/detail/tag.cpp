// galimatron
#include "galimatron/translator_library/transforms/xml2cpp/detail/xml2cpp_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::xml2cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_x2s);
}


namespace tag = tags::tag;

void xml2cpp_t::read_tag_statement(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    std::deque<name_t> labels;
    for (const auto ref : node.child(tag::label_refs.data()).child(tag::label_static.data()).children(tag::label_ref.data()))
    {
        if (const auto name = ref.attribute(tag_c::name.data()))
        {
            labels.push_back(name.value());
        }
    }

    const auto mode = [&]() -> tag_mode_et
    {
        const auto text = attribute_if(node, tag_c::mode);
        if (!text)
        {
            return tag_mode_et::INHERIT;
        }
        if (*text == tags::tag_gal::once)
        {
            return tag_mode_et::ONCE;
        }
        if (*text == tags::tag_gal::local)
        {
            return tag_mode_et::LOCAL;
        }
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ std::format("unknown tag mode '{}'", *text) }
        );
    }();

    const auto vd = builder_.create_tag_statement({
        .version_definition_node = parent,
        .index_in_version = attribute_index(node),
        .name = attribute_if(node, tag_c::name),
        .labels = labels,
        .mode = mode,
        .location = read_location(node)
    });
    restore_layout(node, vd);

    read_tag_values(node.child(tag::values.data()), vd, vd);
}

void xml2cpp_t::read_tag_values(const pugi::xml_node& node, const vertex_descriptor_t parent, const vertex_descriptor_t vd_tag)
{
    const auto vd = builder_.create_tag_values({
        .parent_node = parent
    });

    if (!node)
    {
        return;
    }

    for (const auto child : node.children(tag::value.data()))
    {
        read_tag_value(child, vd, vd_tag);
    }
}

void xml2cpp_t::read_tag_value(const pugi::xml_node& node, const vertex_descriptor_t parent, const vertex_descriptor_t vd_tag)
{
    const auto index = [&] -> index_ot
    {
        if (const auto attr = node.attribute(tag_c::index.data()))
        {
            return std::stoull(attr.value());
        }
        return {};
    }();

    const auto vd = builder_.create_tag_value({
        .parent_node = parent,
        .index = index
    });

    for (const auto child : node.children())
    {
        if (child.type() != pugi::node_element)
        {
            continue;
        }

        const std::string_view name = child.name();
        if (name == tag::record)
        {
            read_tag_record(child, vd, vd_tag);
        }
        else if (name == tag::array)
        {
            read_tag_array(child, vd, vd_tag);
        }
        else if (name == tag_c::string)
        {
            read_tag_string(child, vd, vd_tag, index);
        }
        else
        {
            unexpected(child);
        }
    }
}

void xml2cpp_t::read_tag_record(const pugi::xml_node& node, const vertex_descriptor_t parent, const vertex_descriptor_t vd_tag)
{
    const auto vd = builder_.create_tag_record({
        .parent_node = parent
    });

    for (const auto child : node.children(tag::key.data()))
    {
        read_tag_keyvalue(child, vd, vd_tag);
    }
}

void xml2cpp_t::read_tag_keyvalue(const pugi::xml_node& node, const vertex_descriptor_t parent, const vertex_descriptor_t vd_tag)
{
    const auto vd = builder_.create_tag_keyvalue({
        .parent_node = parent,
        .key = read_cdata(node),
        .key_delimiter = attribute_delimiter(node)
    });

    for (const auto child : node.children(tag::value.data()))
    {
        read_tag_value(child, vd, vd_tag);
    }
}

void xml2cpp_t::read_tag_array(const pugi::xml_node& node, const vertex_descriptor_t parent, const vertex_descriptor_t vd_tag)
{
    const auto vd = builder_.create_tag_array({
        .parent_node = parent
    });

    for (const auto child : node.children(tag::value.data()))
    {
        read_tag_value(child, vd, vd_tag);
    }
}

void xml2cpp_t::read_tag_string(
    const pugi::xml_node& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag,
    const index_ot value_index
)
{
    const auto mark = attribute_if(node, tag_c::template_engine);
    const auto vd = builder_.create_tag_string({
        .parent_node = parent,
        .value = read_cdata(node).value_or(""),
        .delimiter = attribute_delimiter(node),
        .is_template = mark && *mark == tag_c::bool_true
    });

    if (value_index)
    {
        auto& tag_statement = get_entity<tag_statement_t>(builder_.graph()[vd_tag]);
        tag_statement.string_values_at_zero_level[*value_index] = vd;
    }
}

}// namespace galimatron::translator::transforms::xml2cpp
