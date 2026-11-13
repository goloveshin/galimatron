// galimatron
#include "galimatron/translator_library/transforms/xml2cpp/detail/xml2cpp_impl.hpp"
#include "galimatron/common_library/exception_augment.hpp"

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


namespace tag = tags::ordering;

void xml2cpp_t::read_ordering_statement(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto name = attribute_if(node, tag_c::name);
    if (!name)
    {
        const auto location = read_location(node);
        const auto where = exception::augment_where({
            .path = location ? location->path : path_ot{}
        });
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ "ordering has no name" }
        );
    }

    builder::builder_t::create_ordering_statement_t::ordering_parts_t parts;
    for (const auto part_node : node.child(tag::ordering_parts.data()).children(tag::ordering_part.data()))
    {
        builder::builder_t::create_ordering_statement_t::ordering_part_t part;
        for (const auto ref : part_node.children(tag::aggregate_ref.data()))
        {
            if (const auto ref_name = ref.attribute(tag::name.data()))
            {
                part.push_back(ref_name.value());
            }
        }
        parts.push_back(std::move(part));
    }

    const auto vd = builder_.create_ordering_statement({
        .version_definition_node = parent,
        .index = attribute_index(node),
        .name = *name,
        .ordering_parts = std::move(parts),
        .location = read_location(node)
    });
    restore_layout(node, vd);
}

}// namespace galimatron::translator::transforms::xml2cpp
