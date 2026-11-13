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


using namespace ranges;

void listener_t::enterOrderingStatement(g_parser::OrderingStatementContext * ctx)
{
    auto* definition = ctx->orderingDefinition();
    auto get_ordering_parts = [&]
    {
        auto&& range = definition->orderingPart();

        return
            range
            | views::transform([](auto* orderingPartCtx){
                return vector_of_tokens_to_texts(orderingPartCtx->IDENTIFIER());
            })
            | to<builder::builder_t::create_ordering_statement_t::ordering_parts_t>
            ;
    };

    const auto vd_label = builder_.create_ordering_statement({
        .version_definition_node = parent(),
        .index = entity_index_in_version_,
        .name = token_to_text(definition->name),
        .ordering_parts = get_ordering_parts(),
        .location = {}
    });
    assign_user_comments(vd_label, ctx);
    entity_index_in_version_++;
}

}// namespace galimatron::translator::transforms::gal2cpp
