// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state::builder
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_bld);
}


vertex_descriptor_t builder_t::create_root_aggregate(const create_root_aggregate_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::ROOT_AGGREGATE,
        .entity = root_aggregate_t{
            .name = args.name,
            .created_by_wizard = args.created_by_wizard
        },
        .location = args.location,
        .vd_parent = args.chunk_node
    });
    state_.aggregates().add(args.name, vd, true, chunk_path(args.chunk_node), args.chunk_node, {});
    return vd;
}

vertex_descriptor_t builder_t::create_root_aggregate_version_declarations(const create_root_aggregate_version_declarations_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::ROOT_AGGREGATE_VERSION_DECLARATIONS,
        .entity = root_aggregate_version_declarations_t{},
        .location = {},
        .vd_parent = args.root_aggregate_node
    });
    state_.aggregates().note_declarations(args.root_aggregate_node, vd);
    return vd;
}

vertex_descriptor_t builder_t::create_root_aggregate_version_declaration(const create_root_aggregate_version_declaration_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::ROOT_AGGREGATE_VERSION_DECLARATION,
        .entity = root_aggregate_version_declaration_t{
            .index = args.index,
            .status = args.status,
            .comment = args.comment,
            .hash = args.hash
        },
        .location = {},
        .vd_parent = args.versions_node
    });
    const auto vd_aggregate = graph_[args.versions_node].vd_parent;
    state_.versions().add_declaration(
        vd_aggregate,
        args.index,
        vd,
        args.status,
        args.hash,
        args.comment
    );
    return vd;
}

vertex_descriptor_t builder_t::create_root_aggregate_version_definitions(const create_root_aggregate_version_definitions_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::ROOT_AGGREGATE_VERSION_DEFINITIONS,
        .entity = root_aggregate_version_definitions_t{},
        .location = {},
        .vd_parent = args.root_aggregate_node
    });
    state_.aggregates().note_definitions(args.root_aggregate_node, vd);
    return vd;
}

vertex_descriptor_t builder_t::create_root_aggregate_version_definition(const create_root_aggregate_version_definition_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::ROOT_AGGREGATE_VERSION_DEFINITION,
        .entity = root_aggregate_version_definition_t{
            .index = args.index,
            .owner = args.owner,
            .created_by_wizard = args.created_by_wizard,
            .mode = args.mode
        },
        .location = {},
        .vd_parent = args.versions_node
    });
    const auto vd_aggregate = graph_[args.versions_node].vd_parent;
    if (args.index)
    {
        state_.versions().add_definition(
            vd_aggregate,
            *args.index,
            vd
        );
    }
    else
    {
        state_.versions().add_anonymous(vd_aggregate, vd, args.owner);
    }
    return vd;
}

}// namespace galimatron::translator::state::builder
