// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/wizard/tags.hpp"
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/exception_augment.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"

// Boost.LEAF
#include <boost/leaf/handle_errors.hpp>

// global
#include "global/common_library/utils.hpp"

// leaf_spy
#include "captured_leaf_exception.hpp"

// Taskflow
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <taskflow/taskflow.hpp>

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2cpp_validate
{

using namespace galimatron::translator::state;
using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

// create root aggregate context is chunk
//
bool cpp2cpp_validate_t::wizard_create_root_aggregate_context_is_chunk(const wizard::context_et context) const
{
    return context == wizard::context_et::CHUNK;
}

// there is no root aggregate yet
//
bool cpp2cpp_validate_t::wizard_create_root_aggregate_root_aggregate_is_absent() const
{
    return std::ranges::none_of(aggregate_versions_, &aggregate_versions_t::is_root);
}

// checks of the current wizard command, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::may_execute_wizard_command()
{
    const auto& command = wizard_statement(language_part_);
    switch (command.type)
    {
        case wizard::command_et::CREATE_ROOT_AGGREGATE:
            is_wizard_create_root_aggregate_context_chunk();
            is_wizard_create_root_aggregate_command_file();
            is_wizard_create_root_aggregate_chunk_absent();
            is_wizard_create_root_aggregate_root_aggregate_absent();
            return true;
        case wizard::command_et::CREATE_AGGREGATE:
            is_wizard_create_aggregate_context_chunk();
            is_wizard_create_aggregate_root_aggregate_present();
            is_wizard_create_aggregate_last_version_open();
            is_wizard_create_aggregate_command_file();
            is_wizard_create_aggregate_names_unique();
            is_wizard_create_aggregate_chunk_absent();
            return true;
        case wizard::command_et::COPY_AGGREGATE:
            is_wizard_copy_aggregate_context_chunk();
            is_wizard_copy_aggregate_root_aggregate_present();
            is_wizard_copy_aggregate_source_present();
            is_wizard_copy_aggregate_not_root();
            is_wizard_copy_aggregate_names_unique();
            is_wizard_copy_aggregate_aggregates_absent();
            is_wizard_copy_aggregate_command_file();
            is_wizard_copy_aggregate_chunk_absent();
            is_wizard_copy_aggregate_source_open();
            return true;
        case wizard::command_et::CONTINUE_AGGREGATE:
            is_wizard_continue_aggregate_context_chunk();
            is_wizard_continue_aggregate_root_aggregate_present();
            is_wizard_continue_aggregate_source_present();
            is_wizard_continue_aggregate_not_root();
            is_wizard_continue_aggregate_names_unique();
            is_wizard_continue_aggregate_aggregates_absent();
            is_wizard_continue_aggregate_command_file();
            is_wizard_continue_aggregate_chunk_absent();
            is_wizard_continue_aggregate_last_version_live();
            return true;
        case wizard::command_et::DELETE_AGGREGATE:
            is_wizard_delete_aggregate_context_chunk();
            is_wizard_delete_aggregate_root_aggregate_present();
            is_wizard_delete_aggregate_names_unique();
            is_wizard_delete_aggregate_aggregates_exist();
            is_wizard_delete_aggregate_not_root();
            is_wizard_delete_aggregate_last_version_live();
            is_wizard_delete_aggregate_last_version_open();
            is_wizard_delete_aggregate_version_definition_present();
            is_wizard_delete_aggregate_physically_closed();
            is_wizard_delete_aggregate_physically_not_ordered();
            return true;
        case wizard::command_et::CREATE_VERSION:
            is_wizard_create_version_context();
            is_wizard_create_version_root_present();
            return true;
        case wizard::command_et::DELETE_VERSION:
            is_wizard_delete_version_context();
            is_wizard_delete_version_root_present();
            is_wizard_delete_version_present();
            is_wizard_delete_version_open();
            is_wizard_delete_version_continuous();
            return true;
        case wizard::command_et::COLLAPSE_VERSION:
            is_wizard_collapse_version_context();
            is_wizard_collapse_version_root_present();
            is_wizard_collapse_version_present();
            is_wizard_collapse_version_owner_present();
            return true;
        case wizard::command_et::CREATE_LABEL:
        case wizard::command_et::CREATE_TAG:
        case wizard::command_et::DELETE_TAG:
        case wizard::command_et::CREATE_ORDERING:
            is_wizard_version_statement();
            return true;
        case wizard::command_et::CREATE_HASH:
            is_wizard_create_hash_context();
            is_wizard_hash_version_present();
            return true;
        case wizard::command_et::DELETE_HASH:
            is_wizard_delete_hash_context();
            is_wizard_hash_version_present();
            return true;
        case wizard::command_et::BATCH:
            is_wizard_batch_context_chunk();
            return true;
    }

    return true;
}

// root aggregate checks are skipped: it does not exist yet
// (create root aggregate)
//
bool cpp2cpp_validate_t::skips_root_aggregate_checks() const
{
    if (language_part_.wizard_statements().empty())
    {
        return false;
    }

    const auto& command = wizard_statement(language_part_);
    if (command.type == wizard::command_et::CREATE_ROOT_AGGREGATE)
    {
        return true;
    }

    if (command.type == wizard::command_et::BATCH)
    {
        return std::ranges::any_of(command.commands, [](const wizard::batch_command_t& item)
        {
            return item.type == wizard::command_et::CREATE_ROOT_AGGREGATE;
        });
    }

    return false;
}

// among all commands there is exactly one wizard command, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_command_single()
{
    const auto count = language_part_.wizard_statements().size();
    if (count == 1)
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COMMAND_MUST_BE_ONE, std::format("{}: {}", wizard::tags::command::must_be_one, count));
}

// create root aggregate runs in the chunk context, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_root_aggregate_context_chunk()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_ROOT_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_root_aggregate_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_ROOT_AGGREGATE_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::create_root_aggregate::command,
        wizard::tags::create_root_aggregate::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

// create root aggregate writes name.gal next to the command file.
// the command file is not that file, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_root_aggregate_command_file()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_ROOT_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    const auto& name = data.names.front();
    if (!command_file_is_the_aggregate(command.chunk_uri, name, data.directory))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_ROOT_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::create_root_aggregate::command,
        wizard::tags::create_root_aggregate::command_file_is_the_aggregate,
        report_path(command.chunk_uri)
    ), {
        .path = command.chunk_uri,
        .aggregate = name
    }
    );
}

// the new root aggregate file is not already loaded,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_root_aggregate_chunk_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_ROOT_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    const auto& name = data.names.front();
    if (new_aggregate_chunk_is_absent(command.chunk_node, name, data.directory))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_ROOT_AGGREGATE_CHUNK_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::create_root_aggregate::command,
        wizard::tags::create_root_aggregate::chunk_already_exists,
        report_path(new_aggregate_chunk_path(command.chunk_node, name, data.directory))
    ), {
        .path = command.chunk_uri,
        .aggregate = name
    }
    );
}

// there is no root aggregate, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_root_aggregate_root_aggregate_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_ROOT_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_root_aggregate_root_aggregate_is_absent())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_ROOT_AGGREGATE_ROOT_AGGREGATE_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::create_root_aggregate::command,
        wizard::tags::create_root_aggregate::root_aggregate_already_exists,
        join_root_chunk_paths(false)
    ), {
        .path = command.chunk_uri
    }
    );
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
