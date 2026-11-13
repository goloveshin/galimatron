// galimatron
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"

// range-v3
#include <range/v3/iterator/operations.hpp>

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


namespace tag = gal_and_xml_tags::aggregate;

pugi::xml_node cpp2xml_t::create_aggregate(const aggregate_t& args)
{
    auto node = parent().append_child(tag::aggregate);
    {
        node.append_attribute(tag_c::name) = args.name;
        node.append_attribute(tag_c::index) = args.index;
    }

    if (args.created_by_wizard)
    {
        append_wizard_mark(node, gal_and_xml_tags::wizard_mark::new_aggregate);
    }

    aggregate_name_ = args.name;

    return node;
}

pugi::xml_node cpp2xml_t::create_aggregate_version_declarations(const aggregate_version_declarations_t& args)
{
    auto node = parent().append_child(tag::aggregate_version_declarations);

    return node;
}

pugi::xml_node cpp2xml_t::create_aggregate_version_declaration(const aggregate_version_declaration_t& args)
{
    auto node = parent().append_child(tag::aggregate_version_declaration);
    {
        node.append_attribute(tag_c::index) = args.index;

        if (args.status)
        {
            node.append_attribute(tag_c::status) = to_string(args.status.value());
        }

        if (args.comment)
        {
            node.append_attribute(tag_c::comment) = args.comment.value();
        }

        if (args.hash)
        {
            node.append_attribute(tag_c::hash) = args.hash.value();
        }
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_aggregate_version_definitions(const aggregate_version_definitions_t& args)
{
    auto node = parent().append_child(tag::aggregate_version_definitions);

    return node;
}

pugi::xml_node cpp2xml_t::create_aggregate_version_definition(const aggregate_version_definition_t& args)
{
    auto node = parent().append_child(tag::aggregate_version_definition);
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

    if (compiled_state_ && args.index)
    {
        const auto& orderings = compiled_state_.value()->ordering_ranks();

        auto range = orderings.try_find_range_by_aggregate_name_and_version_index(
            boost::make_tuple(*aggregate_name_, *args.index)
        );

        if (ranges::distance(range))
        {
            auto orderings_node = node.append_child(tag::orderings);

            for (const auto& ordering : range)
            {
                auto ordering_node = orderings_node.append_child(tag::ordering);
                {
                    ordering_node.append_attribute(tag_c::name) = ordering.ordering_name;
                    ordering_node.append_attribute(tag_c::index) = ordering.rank;
                }
            }
        }
    }

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
