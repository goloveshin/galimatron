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


namespace tag = tags::root_aggregate;

void xml2cpp_t::read_root_aggregate(const pugi::xml_node& node, const vertex_descriptor_t parent)
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
            exception::e_what_info{ "root aggregate has no name" }
        );
    }

    const auto vd = builder_.create_root_aggregate({
        .chunk_node = parent,
        .name = *name,
        .location = read_location(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

void xml2cpp_t::read_root_aggregate_version_declarations(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_root_aggregate_version_declarations({
        .root_aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto child : node.children(tag::root_aggregate_version_declaration.data()))
    {
        read_root_aggregate_version_declaration(child, vd);
    }
}

void xml2cpp_t::read_root_aggregate_version_declaration(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_root_aggregate_version_declaration({
        .versions_node = parent,
        .index = attribute_index(node),
        .status = attribute_status(node),
        .comment = attribute_if(node, tag_c::comment),
        .hash = attribute_if(node, tag_c::hash)
    });
    restore_layout(node, vd);
}

void xml2cpp_t::read_root_aggregate_version_definitions(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_root_aggregate_version_definitions({
        .root_aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto child : node.children(tag::root_aggregate_version_definition.data()))
    {
        read_root_aggregate_version_definition(child, vd);
    }
    builder_.state().bind_anonymous_owners(vd);
}

void xml2cpp_t::read_root_aggregate_version_definition(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    index_ot owner;
    if (const auto text = attribute_if(node, tag_c::owner))
    {
        owner = index_t{ std::stoull(*text) };
    }
    const auto vd = builder_.create_root_aggregate_version_definition({
        .versions_node = parent,
        .index = attribute_index_if(node),
        .owner = owner,
        .mode = attribute_mode(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

}// namespace galimatron::translator::transforms::xml2cpp
