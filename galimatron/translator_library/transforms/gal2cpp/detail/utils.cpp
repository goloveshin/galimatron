// galimatron
#include "galimatron/translator_library/transforms/gal2cpp/detail/utils.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// Antlr
#include <misc/Interval.h>

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::gal2cpp
{

namespace GaL = galimatron::translator::state::gal_and_xml_tags::GaL;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_g2s);
}


content_ot original_user_formatting(antlr4::ParserRuleContext* context, const std::size_t stop_index)
{
    if (!context)
    {
        return {};
    }

    auto* start = context->getStart();
    if (!start || !start->getInputStream())
    {
        return {};
    }

    const auto start_index = start->getStartIndex();
    if (start_index == INVALID_INDEX || stop_index == INVALID_INDEX || stop_index < start_index)
    {
        return {};
    }

    auto text = start->getInputStream()->getText(antlr4::misc::Interval(start_index, stop_index));
    boost::algorithm::replace_all(text, GaL::CRLF, GaL::NEWLINE);
    boost::algorithm::replace_all(text, GaL::CR, GaL::NEWLINE);
    return text;
}

bool token_to_bool(antlr4::Token* token)
{
    return token;
}

bool token_to_bool(antlr4::tree::TerminalNode* node)
{
    return node;
}

std::string token_to_text(antlr4::Token* token)
{
    return token->getText();
}

state::string_delimiter_t delimiter_of(const g_lexer::string_lexeme_t& lexeme)
{
    return {
        .raw = lexeme.raw,
        .quote = lexeme.quote,
        .dashes = lexeme.dashes
    };
}

string_ot token_to_text_if(antlr4::Token* token)
{
    if (!token) return {};
    return token_to_text(token);
}

unsigned long token_to_ul(antlr4::Token* token)
{
    return std::stoul(token_to_text(token));
}

std::optional<unsigned long> token_to_ul_if(antlr4::Token* token)
{
    return token_to_text_if(token)
        .transform([](const auto str){ return std::stoul(str); });
}

strings_t vector_of_tokens_to_texts(const std::vector<antlr4::tree::TerminalNode*>& nodes)
{
    return
        nodes
        | ranges::views::transform([](auto* node){ return node->getText(); })
        | ranges::to<strings_t>
        ;
}

strings_t vector_of_tokens_to_texts(const std::vector<antlr4::Token*>& nodes)
{
    return
        nodes
        | ranges::views::transform([](auto* node){ return node->getText(); })
        | ranges::to<strings_t>
        ;
}

string_ot to_hash(g_parser::RootAggregateVersionDeclarationHeaderContext* top_ctx)
{
    if (!top_ctx)
    {
        return {};
    }
    for (auto* body : top_ctx->rootAggregateVersionDeclarationHeaderBody())
    {
        if (!body)
        {
            continue;
        }
        auto* hash = body->versionHash();
        if (!hash)
        {
            continue;
        }
        return token_to_text(hash->version_hash);
    }
    return {};
}

string_ot to_hash(g_parser::AggregateVersionDeclarationHeaderContext* top_ctx)
{
    if (!top_ctx)
    {
        return {};
    }
    for (auto* body : top_ctx->aggregateVersionDeclarationHeaderBody())
    {
        if (!body)
        {
            continue;
        }
        auto* hash = body->versionHash();
        if (!hash)
        {
            continue;
        }
        return token_to_text(hash->version_hash);
    }
    return {};
}

string_ot to_comment(g_parser::RootAggregateVersionDeclarationHeaderContext* top_ctx)
{
    if (!top_ctx)
    {
        return {};
    }
    for (auto* body : top_ctx->rootAggregateVersionDeclarationHeaderBody())
    {
        if (!body)
        {
            continue;
        }
        auto* comment = body->versionComment();
        if (!comment)
        {
            continue;
        }
        return token_to_text(comment->version_comment);
    }
    return {};
}

string_ot to_comment(g_parser::AggregateVersionDeclarationHeaderContext* top_ctx)
{
    if (!top_ctx)
    {
        return {};
    }
    for (auto* body : top_ctx->aggregateVersionDeclarationHeaderBody())
    {
        if (!body)
        {
            continue;
        }
        auto* comment = body->versionComment();
        if (!comment)
        {
            continue;
        }
        return token_to_text(comment->version_comment);
    }
    return {};
}

state::version_status_oet to_status(g_parser::AggregateVersionDeclarationHeaderContext* top_ctx)
{
    if (!top_ctx)
    {
        return {};
    }
    return state::to_enum(top_ctx->version_status);
}

state::tag_mode_et tag_mode_of(g_parser::TagModeContext* ctx)
{
    if (!ctx)
    {
        return state::tag_mode_et::INHERIT;
    }
    if (ctx->ONCE())
    {
        return state::tag_mode_et::ONCE;
    }
    return state::tag_mode_et::LOCAL;
}

state::tag_mode_et version_mode_of(g_parser::VersionModeContext* ctx)
{
    if (!ctx)
    {
        return state::tag_mode_et::INHERIT;
    }
    if (ctx->ONCE())
    {
        return state::tag_mode_et::ONCE;
    }
    if (ctx->LOCAL())
    {
        return state::tag_mode_et::LOCAL;
    }
    return state::tag_mode_et::OWN;
}

}// namespace galimatron::translator::transforms::gal2cpp
