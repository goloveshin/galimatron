// galimatron
#include "galimatron/translator_library/parser/explains.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::parser::explains
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_par);
}


namespace
{

struct known_t
{
    std::string_view rule;
    std::string_view message;
    std::string_view text;
};

constexpr known_t known[] =
{
    {
        .rule = "aggregateVersionDefinition",
        .message = "mismatched input '{' expecting {'before', 'once', 'local', 'own', INTEGER}",
        .text = "anonymous version without an owner"
    },
    {
        .rule = "rootAggregateVersionDefinition",
        .message = "mismatched input '{' expecting {'before', 'once', 'local', 'own', INTEGER}",
        .text = "anonymous version without an owner"
    },
    {
        .rule = "aggregateVersionDefinition",
        .message = "extraneous input '=' expecting '{'",
        .text = "a version definition cannot have a header"
    },
    {
        .rule = "rootAggregateVersionDefinition",
        .message = "extraneous input '=' expecting '{'",
        .text = "a version definition cannot have a header"
    },

    {
        .rule = "chunk",
        .message = "no viable alternative at input 'version'",
        .text = "a version cannot come before its aggregate"
    },
    {
        .rule = "content",
        .message = "no viable alternative at input 'version'",
        .text = "a version cannot come before its aggregate"
    },
    {
        .rule = "contentRootAggregateOnly",
        .message = "no viable alternative at input 'version'",
        .text = "a version cannot come before its aggregate"
    },
    {
        .rule = "contentAggregateOnly",
        .message = "no viable alternative at input 'version'",
        .text = "a version cannot come before its aggregate"
    },

    {
        .rule = "productionEntryPoint",
        .message = "mismatched input 'aggregate' expecting <EOF>",
        .text = "a root aggregate and an aggregate cannot share a file"
    },
    {
        .rule = "contentRootAggregateOnly",
        .message = "mismatched input 'aggregate' expecting <EOF>",
        .text = "a root aggregate and an aggregate cannot share a file"
    },

    {
        .rule = "aggregate",
        .message = "mismatched input '}' expecting 'versions'",
        .text = "an aggregate needs versions"
    },
    {
        .rule = "rootAggregate",
        .message = "mismatched input '}' expecting 'versions'",
        .text = "an aggregate needs versions"
    },
    {
        .rule = "aggregateVersionDeclarations",
        .message = "mismatched input '}' expecting 'versions'",
        .text = "an aggregate needs versions"
    },
    {
        .rule = "rootAggregateVersionDeclarations",
        .message = "mismatched input '}' expecting 'versions'",
        .text = "an aggregate needs versions"
    },

    {
        .rule = "aggregateVersionDeclarationHeaderBody",
        .message = "no viable alternative at input 'comment'",
        .text = "a declaration has one comment"
    },
    {
        .rule = "rootAggregateVersionDeclarationHeaderBody",
        .message = "no viable alternative at input 'comment'",
        .text = "a declaration has one comment"
    },
    {
        .rule = "aggregateVersionDeclarationHeader",
        .message = "no viable alternative at input 'comment'",
        .text = "a declaration has one comment"
    },
    {
        .rule = "rootAggregateVersionDeclarationHeader",
        .message = "no viable alternative at input 'comment'",
        .text = "a declaration has one comment"
    },
    {
        .rule = "aggregateVersionDeclarationHeaderBody",
        .message = "no viable alternative at input 'hash'",
        .text = "a declaration has one hash"
    },
    {
        .rule = "rootAggregateVersionDeclarationHeaderBody",
        .message = "no viable alternative at input 'hash'",
        .text = "a declaration has one hash"
    },
    {
        .rule = "aggregateVersionDeclarationHeader",
        .message = "no viable alternative at input 'hash'",
        .text = "a declaration has one hash"
    },
    {
        .rule = "rootAggregateVersionDeclarationHeader",
        .message = "no viable alternative at input 'hash'",
        .text = "a declaration has one hash"
    },

    {
        .rule = "versionDefinitionBody",
        .message = "no viable alternative at input 'version_number;'",
        .text = "a tag needs a value"
    },
    {
        .rule = "rootVersionDefinitionBody",
        .message = "no viable alternative at input 'version_number;'",
        .text = "a tag needs a value"
    },
    {
        .rule = "tagDefinition",
        .message = "no viable alternative at input 'version_number;'",
        .text = "a tag needs a value"
    },
    {
        .rule = "tagStatement",
        .message = "no viable alternative at input 'version_number;'",
        .text = "a tag needs a value"
    },
    {
        .rule = "namedTag",
        .message = "no viable alternative at input 'version_number;'",
        .text = "a tag needs a value"
    },

    {
        .rule = "labelStatic",
        .message = "mismatched input 'dynamic' expecting 'tag-name-from'",
        .text = "a label cannot be static and dynamic at the same time"
    },
    {
        .rule = "labelStatic",
        .message = "mismatched input ',' expecting '}'",
        .text = "tag-name-from names one tag"
    },
    {
        .rule = "labelDynamic",
        .message = "mismatched input ',' expecting '}'",
        .text = "a dynamic label names one value"
    },
    {
        .rule = "labelStatic",
        .message = "missing IDENTIFIER at '}'",
        .text = "a label is missing a name"
    },
    {
        .rule = "labelDynamic",
        .message = "missing IDENTIFIER at '}'",
        .text = "a label is missing a name"
    },

    {
        .rule = "orderingDefinition",
        .message = "mismatched input ';' expecting '='",
        .text = "an ordering needs parts"
    },
    {
        .rule = "orderingStatement",
        .message = "mismatched input ';' expecting '='",
        .text = "an ordering needs parts"
    },
    {
        .rule = "orderingPart",
        .message = "mismatched input '}' expecting IDENTIFIER",
        .text = "an ordering part needs a name"
    },
    {
        .rule = "orderingPart",
        .message = "mismatched input '}' expecting ','",
        .text = "an ordering part needs at least two names"
    }
};

}// namespace


string_ot explain(const strings_t& stack, const std::string& message)
{
    // the failing rule is first. a known rule keeps its own sentence, so a parent does not replace it.
    for (const auto& rule : stack)
    {
        bool known_rule = false;
        for (const auto& row : known)
        {
            if (row.rule != rule)
            {
                continue;
            }
            known_rule = true;
            if (row.message == message)
            {
                return std::string{ row.text };
            }
        }
        if (known_rule)
        {
            return {};
        }
    }
    return {};
}

}// namespace galimatron::translator::parser::explains
