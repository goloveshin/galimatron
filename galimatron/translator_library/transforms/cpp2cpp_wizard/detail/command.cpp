// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// global
#include "global/common_library/utils.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

namespace GaL = gal_and_xml_tags::GaL;

namespace
{
strings_t lines_of(std::string text)
{
    boost::algorithm::replace_all(text, GaL::CRLF, GaL::NEWLINE);
    boost::algorithm::replace_all(text, GaL::CR, GaL::NEWLINE);
    if (!text.empty() && text.back() == GaL::NEWLINE_CHAR)
    {
        text.pop_back();
    }

    strings_t lines;
    boost::split(lines, text, boost::is_any_of(GaL::NEWLINE));
    return lines;
}
}// namespace

cpp2cpp_wizard_t::cpp2cpp_wizard_t(holder::language_part_t& language_state)
    : language_state_(&language_state)
    , validate_memo_(std::make_unique<cpp2cpp_validate::validate_memo_t>())
    , validate_(std::make_unique<cpp2cpp_validate::cpp2cpp_validate_t>(language_state, *validate_memo_))
{}

void cpp2cpp_wizard_t::apply_one(const wizard::command_et type)
{
    switch (type)
    {
        case wizard::command_et::CREATE_ROOT_AGGREGATE:
            apply_wizard_create_root_aggregate();
            break;
        case wizard::command_et::CREATE_AGGREGATE:
            apply_wizard_create_aggregate();
            break;
        case wizard::command_et::COPY_AGGREGATE:
            apply_wizard_copy_aggregate();
            break;
        case wizard::command_et::CONTINUE_AGGREGATE:
            apply_wizard_continue_aggregate();
            break;
        case wizard::command_et::DELETE_AGGREGATE:
            apply_wizard_delete_aggregate();
            break;
        case wizard::command_et::CREATE_VERSION:
            apply_wizard_create_version();
            break;
        case wizard::command_et::DELETE_VERSION:
            apply_wizard_delete_version();
            break;
        case wizard::command_et::COLLAPSE_VERSION:
            apply_wizard_collapse_version();
            break;
        case wizard::command_et::CREATE_LABEL:
            apply_wizard_create_label();
            break;
        case wizard::command_et::CREATE_TAG:
            apply_wizard_create_tag();
            break;
        case wizard::command_et::DELETE_TAG:
            apply_wizard_delete_tag();
            break;
        case wizard::command_et::CREATE_ORDERING:
            apply_wizard_create_ordering();
            break;
        case wizard::command_et::CREATE_HASH:
            apply_wizard_create_hash();
            break;
        case wizard::command_et::DELETE_HASH:
            apply_wizard_delete_hash();
            break;
        case wizard::command_et::BATCH:
            break;
    }
}

void cpp2cpp_wizard_t::refresh_checks()
{
    auto memo = std::make_unique<cpp2cpp_validate::validate_memo_t>();
    auto validate = std::make_unique<cpp2cpp_validate::cpp2cpp_validate_t>(*language_state_, *memo);
    validate_ = std::move(validate);
    validate_memo_ = std::move(memo);
}

void cpp2cpp_wizard_t::apply_wizard_batch()
{
    validate_->may_execute_wizard_command();

    auto& stored = wizard_statement(*language_state_);
    const auto commands = stored.commands;
    const auto saved = stored;

    for (const auto& item : commands)
    {
        refresh_checks();
        stored.type = item.type;
        stored.data = item.data;
        validate_->is_wizard_batch_command_create();
        apply_one(item.type);
    }

    stored = saved;
}

void cpp2cpp_wizard_t::comment_executed_command()
{
    const auto statements = language_state_->wizard_statements();
    if (statements.empty())
    {
        return;
    }

    auto& graph = language_state_->graph();
    auto& vertex = graph[statements.front()];
    const auto& command = std::get<wizard_statement_t>(vertex.entity);
    if (command.chunk_node == vertex_descriptor_not_present)
    {
        return;
    }

    auto& chunk = graph[command.chunk_node];
    const bool has_description = !vertex.user_comments.empty();
    chunk.user_comments.insert(
        chunk.user_comments.end(),
        vertex.user_comments.begin(),
        vertex.user_comments.end()
    );

    if (vertex.user_formatting && !vertex.user_formatting->empty())
    {
        // a description above wizard ends with a blank //, the same way a comment above a function does
        if (has_description && vertex.user_comments.back() != GaL::CPP_COMMENT)
        {
            chunk.user_comments.push_back(std::string{ GaL::CPP_COMMENT });
        }
        auto commented = lines_of(*vertex.user_formatting);
        const auto prefixed =
            commented
            | ranges::views::transform([](const std::string& line)
            {
                return std::string{ GaL::CPP_COMMENT_WITH_SPACE } + line;
            })
            | ranges::to<strings_t>()
            ;
        chunk.user_comments.insert(
            chunk.user_comments.end(),
            prefixed.begin(),
            prefixed.end()
        );
    }

    vertex.user_comments.clear();
    vertex.user_formatting.reset();
}

void cpp2cpp_wizard_t::translate()
{
    validate_->is_wizard_command_single();

    const auto& command = wizard_statement(*language_state_);
    if (command.type == wizard::command_et::BATCH)
    {
        apply_wizard_batch();
    }
    else
    {
        apply_one(command.type);
    }

    comment_executed_command();
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
