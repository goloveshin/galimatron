// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"

// global
#include "global/common_library/utils.hpp"

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


vertex_descriptor_t builder_t::create_operation_statement(const create_operation_statement_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::OPERATION_STATEMENT,
        .entity = operation_statement_t{
            .index_in_version = args.index,
            .name = args.name,
            .is_version = args.is_version
        },
        .location = args.location,
        .vd_parent = args.version_definition_node
    });
    state_.note_version_child(vd);
    return vd;
}

// delete; in an aggregate version definition: the version's other vertices are removed, the declaration is marked deleted
//
void builder_t::apply_aggregate_version_delete(const vertex_descriptor_t vd_definition)
{
    std::vector<vertex_descriptor_t> children;
    for (const auto vd_child : subrange(boost::adjacent_vertices(vd_definition, graph_)))
    {
        children.push_back(vd_child);
    }

    vertex_descriptor_ot kept;
    for (const auto vd_child : children)
    {
        const auto* operation = std::get_if<operation_statement_t>(&graph_[vd_child].entity);
        if (operation && operation->is_version && !kept)
        {
            kept = vd_child;
        }
    }

    if (!kept)
    {
        return;
    }

    for (const auto vd_child : children)
    {
        if (vd_child == *kept)
        {
            continue;
        }

        state_.detach_vertex(vd_definition, vd_child);
    }

    const auto& definition = std::get<aggregate_version_definition_t>(graph_[vd_definition].entity);
    // an anonymous body has no declaration. validation requires delete; when its owner is deleted
    if (!definition.index)
    {
        return;
    }
    const auto vd_definitions = graph_[vd_definition].vd_parent;
    const auto vd_aggregate = graph_[vd_definitions].vd_parent;

    state_.versions().set_status(
        vd_aggregate,
        *definition.index,
        version_status_et::DELETED
    );

    if (const auto vd_declaration = state_.versions().try_declaration(vd_aggregate, *definition.index))
    {
        auto& declaration = std::get<aggregate_version_declaration_t>(graph_[*vd_declaration].entity);
        declaration.status = version_status_et::DELETED;
    }
}

}// namespace galimatron::translator::state::builder
