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


namespace tag = gal_and_xml_tags::ordering;

pugi::xml_node cpp2xml_t::create_ordering_statement(const ordering_statement_t& args)
{
    auto node = parent().append_child(tag::ordering);
    {
        node.append_attribute(tag_c::index) = args.index_in_version;
        node.append_attribute(tag_c::name) = args.name;
    }

    auto parts_node = node.append_child(tag::ordering_parts);
    {
        for (const auto& part : args.ordering_parts)
        {
            auto part_node = parts_node.append_child(tag::ordering_part);

            for (const auto& aggregate_name : part)
            {
                auto aggregate_ref_node = part_node.append_child(tag::aggregate_ref);
                aggregate_ref_node.append_attribute(tag::name) = aggregate_name;
            }
        }
    }

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
