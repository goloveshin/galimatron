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


namespace tag = tags::aggregate;

void xml2cpp_t::read_aggregate(const pugi::xml_node& node, const vertex_descriptor_t parent)
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
            exception::e_what_info{ "aggregate has no name" }
        );
    }

    const auto index = [&] -> index_t
    {
        if (const auto attr = node.attribute(tag_c::index.data()))
        {
            const index_t value = std::stoull(attr.value());
            next_aggregate_index_ = std::max(next_aggregate_index_, value + 1);
            return value;
        }
        return next_aggregate_index_++;
    }();

    const auto vd = builder_.create_aggregate({
        .chunk_node = parent,
        .name = *name,
        .index = index,
        .location = read_location(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

void xml2cpp_t::read_aggregate_version_declarations(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_aggregate_version_declarations({
        .aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto child : node.children(tag::aggregate_version_declaration.data()))
    {
        read_aggregate_version_declaration(child, vd);
    }
}

void xml2cpp_t::read_aggregate_version_declaration(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_aggregate_version_declaration({
        .versions_node = parent,
        .index = attribute_index(node),
        .status = attribute_status(node),
        .comment = attribute_if(node, tag_c::comment),
        .hash = attribute_if(node, tag_c::hash)
    });
    restore_layout(node, vd);
}

void xml2cpp_t::read_aggregate_version_definitions(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_aggregate_version_definitions({
        .aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto child : node.children(tag::aggregate_version_definition.data()))
    {
        read_aggregate_version_definition(child, vd);
    }
    builder_.state().bind_anonymous_owners(vd);
}

void xml2cpp_t::read_aggregate_version_definition(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    index_ot owner;
    if (const auto text = attribute_if(node, tag_c::owner))
    {
        owner = index_t{ std::stoull(*text) };
    }
    const auto vd = builder_.create_aggregate_version_definition({
        .versions_node = parent,
        .index = attribute_index_if(node),
        .owner = owner,
        .mode = attribute_mode(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
    builder_.apply_aggregate_version_delete(vd);
}

}// namespace galimatron::translator::transforms::xml2cpp
