// galimatron
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2xml
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2x);
}


namespace tag = gal_and_xml_tags::root_aggregate;

pugi::xml_node cpp2xml_t::create_root_aggregate(const root_aggregate_t& args)
{
    auto node = parent().append_child(tag::root_aggregate);
    {
        node.append_attribute(tag_c::name) = args.name;
    }

    if (args.created_by_wizard)
    {
        append_wizard_mark(node, gal_and_xml_tags::wizard_mark::new_root_aggregate);
    }

    return node;

}

pugi::xml_node cpp2xml_t::create_root_aggregate_version_declarations(const root_aggregate_version_declarations_t&)
{
    auto node = parent().append_child(tag::root_aggregate_version_declarations);

    return node;
}

pugi::xml_node cpp2xml_t::create_root_aggregate_version_declaration(const root_aggregate_version_declaration_t& args)
{
    auto node = parent().append_child(tag::root_aggregate_version_declaration);
    {
        node.append_attribute(tag_c::index) = args.index;

        if (args.status)
        {
            node.append_attribute(tag_c::status) = to_string(*args.status);
        }

        if (args.comment)
        {
            node.append_attribute(tag_c::comment) = *args.comment;
        }

        if (args.hash)
        {
            node.append_attribute(tag_c::hash) = *args.hash;
        }
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_root_aggregate_version_definitions(const root_aggregate_version_definitions_t&)
{
    auto node = parent().append_child(tag::root_aggregate_version_definitions);

    return node;
}

pugi::xml_node cpp2xml_t::create_root_aggregate_version_definition(const root_aggregate_version_definition_t& args)
{
    auto node = parent().append_child(tag::root_aggregate_version_definition);
    if (args.index)
    {
        node.append_attribute(tag_c::index) = *args.index;
    }
    else if (args.owner)
    {
        node.append_attribute(tag_c::owner) = *args.owner;
    }
    if (args.mode == tag_mode_et::ONCE)
    {
        node.append_attribute(tag_c::mode) = gal_and_xml_tags::tag_gal::once.data();
    }
    else if (args.mode == tag_mode_et::LOCAL)
    {
        node.append_attribute(tag_c::mode) = gal_and_xml_tags::tag_gal::local.data();
    }
    else if (args.mode == tag_mode_et::OWN)
    {
        node.append_attribute(tag_c::mode) = gal_and_xml_tags::tag_gal::own.data();
    }

    if (args.created_by_wizard)
    {
        append_wizard_mark(node, gal_and_xml_tags::wizard_mark::new_version);
    }

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
