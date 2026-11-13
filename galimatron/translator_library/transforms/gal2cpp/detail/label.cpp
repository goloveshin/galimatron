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


void listener_t::enterLabelStatement(g_parser::LabelStatementContext * ctx)
{
    auto* definition = ctx->labelDefinition();
    const auto vd_label = builder_.create_label_statement({
        .version_definition_node = parent(),
        .index = entity_index_in_version_,
        .name = token_to_text(definition->name),
        .location = {}
    });

    if (auto* ctxx = definition->labelStatic())
    {
        builder_.create_label_static({
            .label_node = vd_label,
            .values = vector_of_tokens_to_texts(ctxx->static_values),
            .tag_name_from = token_to_text_if(ctxx->tag_name_from)
        });
    }
    else if (auto* ctxx = definition->labelDynamic())
    {
        builder_.create_label_dynamic({
            .label_node = vd_label,
            .value = token_to_text(ctxx->value)
        });
    }

    assign_user_comments(vd_label, ctx);

    entity_index_in_version_++;
    entity_index_in_versions_++;
}

}// namespace galimatron::translator::transforms::gal2cpp
