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

namespace
{

// a wizard command stores the tag and builds it later. these rules must not add vertices
//
bool inside_wizard_command(antlr4::tree::ParseTree* node)
{
    for (auto* current = node; current; current = current->parent)
    {
        if (dynamic_cast<g_parser::WizardCommandContext*>(current))
        {
            return true;
        }
    }
    return false;
}

}// namespace


void listener_t::enterTagStatement(g_parser::TagStatementContext * ctx)
{
    auto* definition = ctx->tagDefinition();
    auto* named = definition->namedTag();
    auto* labels = named ? named->tagLabels() : definition->tagLabels();
    auto* mode = named ? named->tagMode() : definition->tagMode();
    push_parent(builder_.create_tag_statement({
        .version_definition_node = parent(),
        .index_in_version = entity_index_in_version_,
        .name = named ? name_ot{ token_to_text(named->name) } : name_ot{},
        .labels = vector_of_tokens_to_texts(labels->IDENTIFIER()),
        .mode = tag_mode_of(mode),
        .location = {}
    }));
    tag_statement_ = get_entity<tag_statement_t>(builder_.graph()[parent()]);
    assign_user_comments(parent(), ctx);
}

void listener_t::exitTagStatement(g_parser::TagStatementContext * /*ctx*/)
{
    tag_statement_.reset();
    entity_index_in_version_++;
    entity_index_in_versions_++;
    pop_parent();
}

void listener_t::enterTagValues(g_parser::TagValuesContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    push_parent(builder_.create_tag_values({
        .parent_node = parent()
    }));
    tag_value_index_ = 0;
}

void listener_t::exitTagValues(g_parser::TagValuesContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    tag_value_index_ = 0;
    pop_parent();
}

void listener_t::enterTagJSONValue(g_parser::TagJSONValueContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    push_parent(builder_.create_tag_value({
        .parent_node = parent(),
        .index = (tag_value_level_ == 0)
            ? index_ot{tag_value_index_++}
            : index_ot{}
    }));
    tag_value_level_++;
}

void listener_t::exitTagJSONValue(g_parser::TagJSONValueContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    tag_value_level_--;
    pop_parent();
}

void listener_t::enterTagJSONRecord(g_parser::TagJSONRecordContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    push_parent(builder_.create_tag_record({
        .parent_node = parent()
    }));
}

void listener_t::exitTagJSONRecord(g_parser::TagJSONRecordContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    pop_parent();
}

void listener_t::enterTagJSONKeyValue(g_parser::TagJSONKeyValueContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    const auto key_lexeme = string_lexeme(ctx->key);
    push_parent(builder_.create_tag_keyvalue({
        .parent_node = parent(),
        .key = ctx->key ? string_ot{ key_lexeme.value } : string_ot{},
        .key_delimiter = ctx->key
            ? builder_.state().intern_string_delimiter(delimiter_of(key_lexeme))
            : index_t{ 0 }
    }));
}

void listener_t::exitTagJSONKeyValue(g_parser::TagJSONKeyValueContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    pop_parent();
}

void listener_t::enterTagJSONArray(g_parser::TagJSONArrayContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    push_parent(builder_.create_tag_array({
        .parent_node = parent()
    }));
}

void listener_t::exitTagJSONArray(g_parser::TagJSONArrayContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    pop_parent();
}

void listener_t::enterTagJSONString(g_parser::TagJSONStringContext * ctx)
{
    if (inside_wizard_command(ctx))
    {
        return;
    }

    const auto lexeme = string_lexeme(ctx->string);
    const auto vd_string = builder_.create_tag_string({
        .parent_node = parent(),
        .value = lexeme.value,
        .delimiter = builder_.state().intern_string_delimiter(delimiter_of(lexeme)),
        .is_template = ctx->TEMPLATE_ENGINE() != nullptr
    });

    if (tag_statement_ && (tag_value_level_ == 1))// TODO 1 and -1 look ugly
    {
        tag_statement_.value().get().string_values_at_zero_level[tag_value_index_ - 1] = vd_string;
    }
}

}// namespace galimatron::translator::transforms::gal2cpp
