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


void listener_t::enterVersionDeleteStatement(g_parser::VersionDeleteStatementContext * ctx)
{
    const auto name = ctx->tagName()
        ? name_ot{ token_to_text(ctx->tagName()->name) }
        : name_ot{};
    const auto vd = builder_.create_operation_statement({
        .version_definition_node = parent(),
        .index = entity_index_in_version_,
        .name = name.value_or(""),
        .is_version = !name,
        .location = {}
    });
    assign_user_comments(vd, ctx);

    entity_index_in_version_++;
    entity_index_in_versions_++;
}

}// namespace galimatron::translator::transforms::gal2cpp
