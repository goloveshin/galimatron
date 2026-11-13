// galimatron
#include "galimatron/translator_library/parser/state.hpp"
#include "galimatron/translator_library/parser/explains.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::parser
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_par);
}


state_t::state_t(const content_t& content)
    : input_stream_(content)
    , lexer_(&input_stream_)
    , token_stream_(&lexer_)
    , parser_(&token_stream_)
{
    lexer_.removeErrorListeners();
    lexer_.addErrorListener(this);

    parser_.removeErrorListeners();
    parser_.addErrorListener(this);

    parser_.getInterpreter<antlr4::atn::ParserATNSimulator>()
        ->setPredictionMode(antlr4::atn::PredictionMode::LL_EXACT_AMBIG_DETECTION);
    parser_.setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());
}

void state_t::do_production_parse()
{
    tree_ = parser_.productionEntryPoint();
}

void state_t::do_test_string_parse()
{
    tree_ = parser_.testStringEntryPoint();
}

antlr_parse_tree_t* state_t::tree() const
{
    return tree_;
}

void state_t::syntaxError(
    antlr4::Recognizer* recognizer,
    antlr4::Token* offendingSymbol,
    size_t line,
    size_t charPositionInLine,
    const std::string& message,
    std::exception_ptr e
)
{
    const auto parser = [&]() -> std::optional<antlr4::Parser*>
    {
        if (auto* parser = dynamic_cast<antlr4::Parser*>(recognizer))
        {
            return parser;
        }
        return {};
    }();

    const auto text = parser.and_then([&](antlr4::Parser* parser)
    {
        const auto rules = parser->getRuleInvocationStack();
        const strings_t stack(rules.cbegin(), rules.cend());
        return explains::explain(stack, message);
    });

    throw antlr4::ParseCancellationException(text.value_or(message));
}

}// namespace galimatron::translator::parser
