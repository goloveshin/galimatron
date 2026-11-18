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

// every check in this file that fails throws:
// - exception::domain::translator_library
// - exception::subdomain::translator_library::translator
using namespace galimatron::translator::state;
using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

// checks that continue aggregate runs in the chunk context
// throws: WIZARD_CONTINUE_AGGREGATE_CONTEXT_MUST_BE_CHUNK
// the message names the context
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_context_chunk()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CONTINUE_AGGREGATE_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

// checks that the root aggregate already exists for continue aggregate
// throws: WIZARD_CONTINUE_AGGREGATE_ROOT_AGGREGATE_MUST_EXIST
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_root_aggregate_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CONTINUE_AGGREGATE_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

// checks that the source aggregate already exists
// throws: WIZARD_CONTINUE_AGGREGATE_SOURCE_MUST_EXIST
// the message names the source
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_source_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& source = std::get<wizard::continue_aggregate_data_t>(command.data).source;
    if (language_part_.aggregates().try_by_name(source))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CONTINUE_AGGREGATE_SOURCE_MUST_EXIST, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::source_must_exist,
        source
    ), {
        .path = command.chunk_uri,
        .aggregate = source
    }
    );
}

// checks that the source is not the root aggregate
// throws: WIZARD_CONTINUE_AGGREGATE_CANNOT_CONTINUE_ROOT_AGGREGATE
// the message names the source
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_not_root()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& source = std::get<wizard::continue_aggregate_data_t>(command.data).source;
    const auto* aggregate = language_part_.aggregates().try_by_name(source);
    if (!aggregate || !aggregate->is_root)
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CONTINUE_AGGREGATE_CANNOT_CONTINUE_ROOT_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::cannot_continue_root_aggregate,
        source
    ), {
        .path = command.chunk_uri,
        .aggregate = source
    }
    );
}

// checks that the new names differ, ignoring case, and differ from the source
// throws: WIZARD_CONTINUE_AGGREGATE_NAMES_MUST_BE_UNIQUE
// the message names the duplicated names
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_names_unique()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::continue_aggregate_data_t>(command.data);
    strings_t names = data.names;
    names.push_back(data.source);

    strings_t duplicated;
    for (auto it = names.begin(); it != names.end(); ++it)
    {
        const auto first = std::find_if(names.begin(), it, [&](const auto& kept)
        {
            return boost::iequals(kept, *it);
        });
        if (first == it || name_already_listed(duplicated, *first))
        {
            continue;
        }
        duplicated.push_back(*first);
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

    fail(WIZARD_CONTINUE_AGGREGATE_NAMES_MUST_BE_UNIQUE, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::names_must_be_unique,
        boost::join(duplicated, ",")
    ), where
    );
}

// checks that each new name is not an aggregate yet
// throws: WIZARD_CONTINUE_AGGREGATE_AGGREGATE_MUST_BE_ABSENT
// the message names the existing aggregates
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_aggregates_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& names = std::get<wizard::continue_aggregate_data_t>(command.data).names;
    strings_t present;
    for (const auto& name : names)
    {
        if (!language_part_.aggregates().try_by_name(name) || name_already_listed(present, name))
        {
            continue;
        }
        present.push_back(name);
    }

    if (present.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (present.size() == 1)
    {
        where.aggregate = present.front();
    }

    fail(WIZARD_CONTINUE_AGGREGATE_AGGREGATE_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::aggregate_must_be_absent,
        boost::join(present, ",")
    ), where
    );
}

// checks that the command file is not one of the new files
// throws: WIZARD_CONTINUE_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE
// the message names the command file
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_command_file()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::continue_aggregate_data_t>(command.data);
    strings_t blocked;
    for (const auto& name : data.names)
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

    fail(WIZARD_CONTINUE_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::command_file_is_the_aggregate,
        report_path(command.chunk_uri)
    ), where
    );
}

// checks that each new chunk file does not exist yet
// throws: WIZARD_CONTINUE_AGGREGATE_CHUNK_MUST_BE_ABSENT
// the message names the existing chunk files
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_chunk_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::continue_aggregate_data_t>(command.data);
    strings_t conflicts;
    for (const auto& name : data.names)
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
    if (data.names.size() == 1)
    {
        where.aggregate = data.names.front();
    }

    fail(WIZARD_CONTINUE_AGGREGATE_CHUNK_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::chunk_already_exists,
        boost::join(conflicts, ",")
    ), where
    );
}

// checks that the source's last version is live
// throws, and the message names the source:
// - WIZARD_CONTINUE_AGGREGATE_LAST_VERSION_MUST_EXIST: the source has no version declaration
// - WIZARD_CONTINUE_AGGREGATE_LAST_VERSION_IS_ALREADY_DELETED: that version is deleted
//
bool cpp2cpp_validate_t::is_wizard_continue_aggregate_last_version_live()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return true;
    }

    const auto& source = std::get<wizard::continue_aggregate_data_t>(command.data).source;
    const auto* aggregate = language_part_.aggregates().try_by_name(source);
    if (!aggregate || aggregate->is_root)
    {
        return true;
    }

    const auto declarations = language_part_.versions().declarations_in_order(aggregate->vd);
    using enum exception::details::validation_error_code_et;

    if (declarations.empty())
    {
        fail(WIZARD_CONTINUE_AGGREGATE_LAST_VERSION_MUST_EXIST, std::format(
            "{}: {}: {}",
            wizard::tags::continue_aggregate::command,
            wizard::tags::continue_aggregate::last_version_must_exist,
            source
        ), {
            .path = command.chunk_uri,
            .aggregate = source
        }
        );
    }

    if (declarations.back().status != version_status_et::DELETED)
    {
        return true;
    }

    fail(WIZARD_CONTINUE_AGGREGATE_LAST_VERSION_IS_ALREADY_DELETED, std::format(
        "{}: {}: {}",
        wizard::tags::continue_aggregate::command,
        wizard::tags::continue_aggregate::last_version_is_already_deleted,
        source
    ), {
        .path = command.chunk_uri,
        .aggregate = source,
        .version = declarations.back().index
    }
    );
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
