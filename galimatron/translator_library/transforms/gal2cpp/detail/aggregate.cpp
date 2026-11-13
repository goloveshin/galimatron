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


void listener_t::enterAggregate(g_parser::AggregateContext * ctx)
{
    push_parent(builder_.create_aggregate({
        .chunk_node = parent(),
        .name = token_to_text(ctx->name),
        .index = config_.aggregate_index++,
        .location = {}
    }));
    vd_aggregate_ = parent();
    entity_index_in_versions_ = 0;
    assign_user_comments(parent(), ctx);
}

void listener_t::exitAggregate(g_parser::AggregateContext * /*ctx*/)
{
    pop_parent();
}

void listener_t::enterAggregateVersionDeclarations(g_parser::AggregateVersionDeclarationsContext * ctx)
{
    push_parent(builder_.create_aggregate_version_declarations({
        .aggregate_node = parent()
    }));
    assign_user_comments(parent(), ctx);
}

void listener_t::exitAggregateVersionDeclarations(g_parser::AggregateVersionDeclarationsContext * /*ctx*/)
{
    pop_parent();
}

void listener_t::enterAggregateVersionDeclaration(g_parser::AggregateVersionDeclarationContext * ctx)
{
    const auto vd = builder_.create_aggregate_version_declaration({
        .versions_node = parent(),
        .index = token_to_ul(ctx->version_index),
        .status = to_status(ctx->aggregateVersionDeclarationHeader()),
        .comment = to_comment(ctx->aggregateVersionDeclarationHeader()),
        .hash = to_hash(ctx->aggregateVersionDeclarationHeader())
    });
    assign_user_comments(vd, ctx);
}

void listener_t::enterAggregateVersionDefinitions(g_parser::AggregateVersionDefinitionsContext * /*ctx*/)
{
    push_parent(builder_.create_aggregate_version_definitions({
        .aggregate_node = *vd_aggregate_
    }));
}
void listener_t::exitAggregateVersionDefinitions(g_parser::AggregateVersionDefinitionsContext * /*ctx*/)
{
    builder_.state().bind_anonymous_owners(parent());
    pop_parent();
}

void listener_t::enterAggregateVersionDefinition(g_parser::AggregateVersionDefinitionContext * ctx)
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
    push_parent(builder_.create_aggregate_version_definition({
        .versions_node = parent(),
        .index = index,
        .owner = owner,
        .mode = version_mode_of(ctx->versionMode())
    }));
    entity_index_in_version_ = 0;
    assign_user_comments(parent(), ctx);
}

void listener_t::exitAggregateVersionDefinition(g_parser::AggregateVersionDefinitionContext * /*ctx*/)
{
    builder_.apply_aggregate_version_delete(parent());
    entity_index_in_version_ = 0;
    pop_parent();
}

}// namespace galimatron::translator::transforms::gal2cpp
