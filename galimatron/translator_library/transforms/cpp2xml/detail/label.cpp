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


namespace tag = gal_and_xml_tags::label;

pugi::xml_node cpp2xml_t::create_label_statement(const label_statement_t& args)
{
    auto node = parent().append_child(tag::label);
    {
        node.append_attribute(tag_c::index) = args.index_in_version;
        node.append_attribute(tag_c::name) = args.name;
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_label_dynamic(const label_dynamic_t& args)
{
    auto node = parent().append_child(tag::dynamic_highlight);
    {
        node.append_attribute(tag_c::value) = args.value;
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_label_static(const label_static_t& args)
{
    auto node = parent().append_child(tag::static_highlight);

    auto values_node = node.append_child(tag::values);
    {
        for (const auto& [index, value] : ranges::views::enumerate(args.values))
        {
            auto value_node = values_node.append_child(tag::value);
            {
                value_node.append_attribute(tag_c::index) = index;
                value_node.append_attribute(tag::value) = value;
            }
        }
    }

    if (args.tag_name_from)
    {
        auto value_node = node.append_child(tag::tag_name_from);
        {
            value_node.append_attribute(tag::value) = *args.tag_name_from;
        }
    }

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
