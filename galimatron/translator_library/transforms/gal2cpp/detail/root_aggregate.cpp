// galimatron
#include "galimatron/translator_library/transforms/gal2cpp/detail/listener.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::gal2cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_g2s);
}


void listener_t::enterRootAggregate(g_parser::RootAggregateContext * ctx)
{
    push_parent(builder_.create_root_aggregate({
        .chunk_node = parent(),
        .name = token_to_text(ctx->name),
        .location = {}
    }));
    vd_aggregate_ = parent();
    entity_index_in_versions_ = 0;
    assign_user_comments(parent(), ctx);
}

void listener_t::exitRootAggregate(g_parser::RootAggregateContext * /*ctx*/)
{
    pop_parent();
}

void listener_t::enterRootAggregateVersionDeclarations(g_parser::RootAggregateVersionDeclarationsContext * ctx)
{
    push_parent(builder_.create_root_aggregate_version_declarations({
        .root_aggregate_node = parent()
    }));
    assign_user_comments(parent(), ctx);
}

void listener_t::exitRootAggregateVersionDeclarations(g_parser::RootAggregateVersionDeclarationsContext * /*ctx*/)
{
    pop_parent();
}

void listener_t::enterRootAggregateVersionDeclaration(g_parser::RootAggregateVersionDeclarationContext * ctx)
{
    const auto* header = ctx->rootAggregateVersionDeclarationHeader();
    const auto vd = builder_.create_root_aggregate_version_declaration({
        .versions_node = parent(),
        .index = token_to_ul(ctx->version_index),
        .status = header ? to_enum(header->version_status) : version_status_oet{},
        .comment = to_comment(ctx->rootAggregateVersionDeclarationHeader()),
        .hash = to_hash(ctx->rootAggregateVersionDeclarationHeader())
    });
    assign_user_comments(vd, ctx);
}

void listener_t::enterRootAggregateVersionDefinitions(g_parser::RootAggregateVersionDefinitionsContext * /*ctx*/)
{
    push_parent(builder_.create_root_aggregate_version_definitions({
        .root_aggregate_node = *vd_aggregate_
    }));
}

void listener_t::exitRootAggregateVersionDefinitions(g_parser::RootAggregateVersionDefinitionsContext * /*ctx*/)
{
    builder_.state().bind_anonymous_owners(parent());
    pop_parent();
}

void listener_t::enterRootAggregateVersionDefinition(g_parser::RootAggregateVersionDefinitionContext * ctx)
{
    index_ot index;
    if (const auto value = token_to_ul_if(ctx->version_index))
    {
        index = index_t{ *value };
    }
    index_ot owner;
    if (const auto value = token_to_ul_if(ctx->owner_index))
    {
        owner = index_t{ *value };
    }
    push_parent(builder_.create_root_aggregate_version_definition({
        .versions_node = parent(),
        .index = index,
        .owner = owner,
        .mode = version_mode_of(ctx->versionMode())
    }));
    entity_index_in_version_ = 0;
    assign_user_comments(parent(), ctx);
}

void listener_t::exitRootAggregateVersionDefinition(g_parser::RootAggregateVersionDefinitionContext * /*ctx*/)
{
    entity_index_in_version_ = 0;
    pop_parent();
}

}// namespace galimatron::translator::transforms::gal2cpp
