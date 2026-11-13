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

// create aggregate runs in the chunk context, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_aggregate_context_chunk()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_AGGREGATE_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::create_aggregate::command,
        wizard::tags::create_aggregate::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

// the last root version is open. create aggregate writes a definition of that version,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_aggregate_last_version_open()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_last_version_is_open())
    {
        return true;
    }

    const auto& root = root_versions();
    const auto& declarations = root.declarations;

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_AGGREGATE_LAST_VERSION_MUST_BE_OPEN, std::format(
        "{}: {}",
        wizard::tags::create_aggregate::command,
        wizard::tags::create_aggregate::last_version_must_be_open
    ), {
        .path = command.chunk_uri,
        .aggregate = root.name,
        .version = declarations.back().index
    }
    );
}

// for create aggregate the root aggregate already exists, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_aggregate_root_aggregate_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_AGGREGATE_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::create_aggregate::command,
        wizard::tags::create_aggregate::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

// create aggregate writes each name.gal next to the command file, or under the named directory.
// the command file is not one of those files, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_aggregate_command_file()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    const auto& names = data.names;
    strings_t blocked;
    for (const auto& name : names)
    {
        if (!command_file_is_the_aggregate(command.chunk_uri, name, data.directory) || name_already_listed(blocked, name))
        {
            continue;
        }
        blocked.push_back(name);
    }

    if (blocked.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (blocked.size() == 1)
    {
        where.aggregate = blocked.front();
    }

    fail(WIZARD_CREATE_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::create_aggregate::command,
        wizard::tags::create_aggregate::command_file_is_the_aggregate,
        report_path(command.chunk_uri)
    ), where
    );
}

// names in one create aggregate command differ, ignoring case,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_aggregate_names_unique()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return true;
    }

    const auto& names = std::get<wizard::create_aggregate_data_t>(command.data).names;
    strings_t seen;
    strings_t duplicated;
    for (const auto& name : names)
    {
        if (!name_already_listed(seen, name))
        {
            seen.push_back(name);
            continue;
        }

        const auto first = std::ranges::find_if(seen, [&](const name_t& kept)
        {
            return boost::iequals(kept, name);
        });
        if (!name_already_listed(duplicated, *first))
        {
            duplicated.push_back(*first);
        }
    }

    if (duplicated.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (duplicated.size() == 1)
    {
        where.aggregate = duplicated.front();
    }

    fail(WIZARD_CREATE_AGGREGATE_NAMES_MUST_BE_UNIQUE, std::format(
        "{}: {}: {}",
        wizard::tags::create_aggregate::command,
        wizard::tags::create_aggregate::names_must_be_unique,
        boost::join(duplicated, ",")
    ), where
    );
}

// each new chunk file does not exist yet, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_create_aggregate_chunk_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    const auto& names = data.names;
    strings_t conflicts;
    for (const auto& name : names)
    {
        if (new_aggregate_chunk_is_absent(command.chunk_node, name, data.directory))
        {
            continue;
        }
        conflicts.push_back(report_path(new_aggregate_chunk_path(command.chunk_node, name, data.directory)));
    }

    if (conflicts.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (names.size() == 1)
    {
        where.aggregate = names.front();
    }

    fail(WIZARD_CREATE_AGGREGATE_CHUNK_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::create_aggregate::command,
        wizard::tags::create_aggregate::chunk_already_exists,
        boost::join(conflicts, ",")
    ), where
    );
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
