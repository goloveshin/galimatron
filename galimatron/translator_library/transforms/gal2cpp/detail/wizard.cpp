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

wizard::context_et wizard_context_of(const entity_et entity)
{
    switch (entity)
    {
        case entity_et::CHUNK:
            return wizard::context_et::CHUNK;
        case entity_et::ROOT_AGGREGATE:
            return wizard::context_et::ROOT_AGGREGATE;
        case entity_et::ROOT_AGGREGATE_VERSION_DECLARATIONS:
            return wizard::context_et::ROOT_AGGREGATE_VERSION_DECLARATIONS;
        case entity_et::ROOT_AGGREGATE_VERSION_DEFINITION:
            return wizard::context_et::ROOT_AGGREGATE_VERSION_DEFINITION;
        case entity_et::AGGREGATE:
            return wizard::context_et::AGGREGATE;
        case entity_et::AGGREGATE_VERSION_DECLARATIONS:
            return wizard::context_et::AGGREGATE_VERSION_DECLARATIONS;
        case entity_et::AGGREGATE_VERSION_DEFINITION:
            return wizard::context_et::AGGREGATE_VERSION_DEFINITION;
        default:
            break;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::gal2cpp{},
        exception::e_what_info{ std::format("wizard command is outside of a wizard context: {}", to_string(entity)) }
    );
}

struct wizard_place_t
{
    vertex_descriptor_t place_node;
    wizard::context_et context;
    vertex_descriptor_t chunk_node;
};

wizard_place_t wizard_place(
    builder::builder_t& builder,
    const vertex_descriptor_t place_node
)
{
    const auto& place = builder.graph()[place_node];
    auto chunk_node = place_node;
    while (builder.graph()[chunk_node].entity_type != entity_et::CHUNK)
    {
        chunk_node = builder.graph()[chunk_node].vd_parent;
    }

    return {
        .place_node = place_node,
        .context = wizard_context_of(place.entity_type),
        .chunk_node = chunk_node
    };
}

// aggregate names written after create aggregate
//
strings_t names_of(const std::vector<antlr4::Token*>& tokens)
{
    return
        tokens
        | ranges::views::transform([](auto* token){ return token_to_text(token); })
        | ranges::to<strings_t>()
        ;
}

// one tag value: a string, a record, or an array
//
wizard::json_t json_of(
    g_parser::TagJSONValueContext* ctx,
    holder::language_part_t& language,
    const g_lexer& lexer
)
{
    if (auto* text = ctx->tagJSONString())
    {
        const auto lexeme = lexer.string_lexeme_at(text->string->getStartIndex());
        return {
            .string = lexeme.value,
            .delimiter = language.intern_string_delimiter(delimiter_of(lexeme)),
            .is_template = text->TEMPLATE_ENGINE() != nullptr
        };
    }
    if (auto* array = ctx->tagJSONArray())
    {
        const auto values = array->tagJSONValue();
        return {
            .is_array = true,
            .elements =
                values
                | ranges::views::transform([&](auto* value){ return json_of(value, language, lexer); })
                | ranges::to<std::vector<wizard::json_t>>()
        };
    }

    const auto pairs = ctx->tagJSONRecord()->tagJSONKeyValue();
    return {
        .elements =
            pairs
            | ranges::views::transform([&](auto* pair)
            {
                auto element = json_of(pair->tagJSONValue(), language, lexer);
                if (pair->key)
                {
                    const auto key_lexeme = lexer.string_lexeme_at(pair->key->getStartIndex());
                    element.key = key_lexeme.value;
                    element.key_delimiter = language.intern_string_delimiter(delimiter_of(key_lexeme));
                }
                return element;
            })
            | ranges::to<std::vector<wizard::json_t>>()
    };
}

// values inside a tag's braces
//
std::vector<wizard::json_t> values_of(
    g_parser::TagValuesContext* ctx,
    holder::language_part_t& language,
    const g_lexer& lexer
)
{
    const auto values = ctx->tagJSONValue();
    return
        values
        | ranges::views::transform([&](auto* value){ return json_of(value, language, lexer); })
        | ranges::to<std::vector<wizard::json_t>>()
        ;
}

// label names written on a tag
//
strings_t labels_of(g_parser::TagLabelsContext* ctx)
{
    const auto tokens = ctx->IDENTIFIER();
    return
        tokens
        | ranges::views::transform([](auto* token){ return token->getText(); })
        | ranges::to<strings_t>()
        ;
}

// one ordering group, the names inside one pair of braces
//
strings_t ordering_part_of(g_parser::OrderingPartContext* ctx)
{
    const auto tokens = ctx->IDENTIFIER();
    return
        tokens
        | ranges::views::transform([](auto* token){ return token->getText(); })
        | ranges::to<strings_t>()
        ;
}

// the aggregate token is absent when the command sits in a version definition
//
name_ot aggregate_of(antlr4::Token* token)
{
    if (!token)
    {
        return {};
    }
    return token_to_text(token);
}

// version numbers written after create hash, delete hash, or delete version
//
indices_t version_indexes_of(const std::vector<antlr4::Token*>& tokens)
{
    return
        tokens
        | ranges::views::transform([](auto* token){ return index_t{ token_to_ul(token) }; })
        | ranges::to<indices_t>()
        ;
}

wizard::batch_command_t command_of(
    g_parser::WizardCommandContext* ctx,
    holder::language_part_t& language,
    const g_lexer& lexer
)
{
    if (const auto* create = ctx->wizardCommandCreateRootAggregate())
    {
        return {
            .type = wizard::command_et::CREATE_ROOT_AGGREGATE,
            .data = wizard::create_aggregate_data_t{
                .names = strings_t{ token_to_text(create->name) }
            }
        };
    }
    if (auto* create = ctx->wizardCommandCreateAggregate())
    {
        return {
            .type = wizard::command_et::CREATE_AGGREGATE,
            .data = wizard::create_aggregate_data_t{
                .names = names_of(create->names),
                .directory = aggregate_of(create->directory),
                .mode = version_mode_of(create->versionMode())
            }
        };
    }
    if (const auto* copy = ctx->wizardCommandCopyAggregate())
    {
        return {
            .type = wizard::command_et::COPY_AGGREGATE,
            .data = wizard::copy_aggregate_data_t{
                .source = token_to_text(copy->source),
                .names = names_of(copy->names),
                .directory = aggregate_of(copy->directory)
            }
        };
    }
    if (const auto* continued = ctx->wizardCommandContinueAggregate())
    {
        return {
            .type = wizard::command_et::CONTINUE_AGGREGATE,
            .data = wizard::continue_aggregate_data_t{
                .source = token_to_text(continued->source),
                .names = names_of(continued->names),
                .directory = aggregate_of(continued->directory)
            }
        };
    }
    if (const auto* erase = ctx->wizardCommandDeleteAggregate())
    {
        return {
            .type = wizard::command_et::DELETE_AGGREGATE,
            .data = wizard::create_aggregate_data_t{
                .names = names_of(erase->names),
                .physically = erase->physically != nullptr
            }
        };
    }
    if (ctx->wizardCommandCreateVersion())
    {
        return {
            .type = wizard::command_et::CREATE_VERSION,
            .data = wizard::create_version_data_t{
                .mode = version_mode_of(ctx->wizardCommandCreateVersion()->versionMode())
            }
        };
    }
    if (const auto* erase = ctx->wizardCommandDeleteVersion())
    {
        return {
            .type = wizard::command_et::DELETE_VERSION,
            .data = wizard::create_version_data_t{
                .version_indexes = version_indexes_of(erase->version_indexes)
            }
        };
    }
    if (const auto* collapse = ctx->wizardCommandCollapseVersion())
    {
        return {
            .type = wizard::command_et::COLLAPSE_VERSION,
            .data = wizard::collapse_version_data_t{
                .first = index_t{ token_to_ul(collapse->first) },
                .last = index_t{ token_to_ul(collapse->last) }
            }
        };
    }
    if (const auto* create_hash = ctx->wizardCommandCreateHash())
    {
        return {
            .type = wizard::command_et::CREATE_HASH,
            .data = wizard::hash_data_t{
                .version_indexes = version_indexes_of(create_hash->version_indexes),
                .force = create_hash->force != nullptr
            }
        };
    }
    if (const auto* delete_hash = ctx->wizardCommandDeleteHash())
    {
        return {
            .type = wizard::command_et::DELETE_HASH,
            .data = wizard::hash_data_t{
                .version_indexes = version_indexes_of(delete_hash->version_indexes),
                .force = delete_hash->force != nullptr
            }
        };
    }
    if (auto* create = ctx->wizardCommandCreateLabel())
    {
        auto* definition = create->labelDefinition();
        auto* dynamic = definition->labelDynamic();
        auto* label_static = definition->labelStatic();
        return {
            .type = wizard::command_et::CREATE_LABEL,
            .data = wizard::label_data_t{
                .aggregate = aggregate_of(create->aggregate_name),
                .name = token_to_text(definition->name),
                .is_dynamic = dynamic != nullptr,
                .static_values = dynamic ? strings_t{} : names_of(label_static->static_values),
                .tag_name_from = dynamic ? name_ot{} : token_to_text_if(label_static->tag_name_from),
                .dynamic_value = dynamic ? name_ot{ token_to_text(dynamic->value) } : name_ot{}
            }
        };
    }
    if (auto* create = ctx->wizardCommandCreateTag())
    {
        if (auto* unnamed = create->unnamedOnAggregate())
        {
            std::vector<antlr4::Token*> tokens{ unnamed->first };
            tokens.insert(tokens.end(), unnamed->rest.begin(), unnamed->rest.end());
            return {
                .type = wizard::command_et::CREATE_TAG,
                .data = wizard::create_tag_data_t{
                    .aggregate = aggregate_of(create->aggregate_name),
                    .name = {},
                    .labels = names_of(tokens),
                    .mode = tag_mode_of(unnamed->tagMode()),
                    .values = values_of(unnamed->tagValues(), language, lexer)
                }
            };
        }
        auto* named = create->namedTag();
        auto* definition = create->tagDefinition();
        if (!named)
        {
            named = definition->namedTag();
        }
        auto* labels = named ? named->tagLabels() : definition->tagLabels();
        auto* mode = named ? named->tagMode() : definition->tagMode();
        auto* values = named ? named->tagValues() : definition->tagValues();
        return {
            .type = wizard::command_et::CREATE_TAG,
            .data = wizard::create_tag_data_t{
                .aggregate = aggregate_of(create->aggregate_name),
                .name = named ? name_ot{ token_to_text(named->name) } : name_ot{},
                .labels = labels_of(labels),
                .mode = tag_mode_of(mode),
                .values = values_of(values, language, lexer)
            }
        };
    }
    if (auto* erase = ctx->wizardCommandDeleteTag())
    {
        return {
            .type = wizard::command_et::DELETE_TAG,
            .data = wizard::delete_tag_data_t{
                .aggregate = aggregate_of(erase->aggregate_name),
                .name = token_to_text(erase->tagName()->name)
            }
        };
    }
    if (auto* create = ctx->wizardCommandCreateOrdering())
    {
        auto* definition = create->orderingDefinition();
        const auto parts = definition->orderingPart();
        return {
            .type = wizard::command_et::CREATE_ORDERING,
            .data = wizard::ordering_data_t{
                .aggregate = aggregate_of(create->aggregate_name),
                .name = token_to_text(definition->name),
                .parts =
                    parts
                    | ranges::views::transform([](auto* part){ return ordering_part_of(part); })
                    | ranges::to<std::vector<strings_t>>()
            }
        };
    }
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::gal2cpp{},
        exception::e_what_info{ "unknown wizard command" }
    );
}

}// namespace

void listener_t::enterWizardCommand(g_parser::WizardCommandContext * ctx)
{
    // the batch listener creates the one statement. a command inside it is not its own vertex
    if (dynamic_cast<g_parser::WizardBatchContext*>(ctx->parent))
    {
        return;
    }

    // the command vertex does not become a parent: it has no children
    const auto place = wizard_place(builder_, parent());
    const auto command = command_of(ctx, builder_.state(), lexer());
    const auto vd = builder_.create_wizard_statement({
        .place_node = place.place_node,
        .type = command.type,
        .context = place.context,
        .chunk_node = place.chunk_node,
        .chunk_uri = config_.content_path,
        .data = command.data
    });
    // the command rule starts after wizard and stops before the semicolon.
    // the statement around it is what cpp2gal comments
    assign_user_comments(vd, dynamic_cast<antlr4::ParserRuleContext*>(ctx->parent));
}

void listener_t::enterWizardBatch(g_parser::WizardBatchContext * ctx)
{
    const auto place = wizard_place(builder_, parent());
    const auto contexts = ctx->wizardCommand();
    auto commands =
        contexts
        | ranges::views::transform([&](auto* command){ return command_of(command, builder_.state(), lexer()); })
        | ranges::to_vector
        ;

    const auto vd = builder_.create_wizard_statement({
        .place_node = place.place_node,
        .type = wizard::command_et::BATCH,
        .context = place.context,
        .chunk_node = place.chunk_node,
        .chunk_uri = config_.content_path,
        // the commands live in commands. command_data_t has no batch alternative
        .data = wizard::create_version_data_t{},
        .commands = std::move(commands)
    });
    assign_user_comments(vd, ctx);
}

}// namespace galimatron::translator::transforms::gal2cpp
