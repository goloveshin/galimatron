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

// copy aggregate runs in the chunk context, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_context_chunk()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COPY_AGGREGATE_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

// for copy aggregate the root aggregate already exists, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_root_aggregate_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COPY_AGGREGATE_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

// the source aggregate already exists, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_source_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto& source = std::get<wizard::copy_aggregate_data_t>(command.data).source;
    if (language_part_.aggregates().try_by_name(source))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COPY_AGGREGATE_SOURCE_MUST_EXIST, std::format(
        "{}: {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::source_must_exist,
        source
    ), {
        .path = command.chunk_uri,
        .aggregate = source
    }
    );
}

// the source is not the root aggregate, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_not_root()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto& source = std::get<wizard::copy_aggregate_data_t>(command.data).source;
    const auto* aggregate = language_part_.aggregates().try_by_name(source);
    if (!aggregate || !aggregate->is_root)
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COPY_AGGREGATE_CANNOT_COPY_ROOT_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::cannot_copy_root_aggregate,
        source
    ), {
        .path = command.chunk_uri,
        .aggregate = source
    }
    );
}

// the new names differ, ignoring case, and differ from the source.
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_names_unique()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::copy_aggregate_data_t>(command.data);
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

    fail(WIZARD_COPY_AGGREGATE_NAMES_MUST_BE_UNIQUE, std::format(
        "{}: {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::names_must_be_unique,
        boost::join(duplicated, ",")
    ), where
    );
}

// each new name is not an aggregate yet, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_aggregates_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto& names = std::get<wizard::copy_aggregate_data_t>(command.data).names;
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

    fail(WIZARD_COPY_AGGREGATE_AGGREGATE_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::aggregate_must_be_absent,
        boost::join(present, ",")
    ), where
    );
}

// the command file is not one of the new files, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_command_file()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::copy_aggregate_data_t>(command.data);
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

    fail(WIZARD_COPY_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::command_file_is_the_aggregate,
        report_path(command.chunk_uri)
    ), where
    );
}

// each new chunk file does not exist yet, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_chunk_absent()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::copy_aggregate_data_t>(command.data);
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

    fail(WIZARD_COPY_AGGREGATE_CHUNK_MUST_BE_ABSENT, std::format(
        "{}: {}: {}",
        wizard::tags::copy_aggregate::command,
        wizard::tags::copy_aggregate::chunk_already_exists,
        boost::join(conflicts, ",")
    ), where
    );
}

// a source still present in a closed version would add a live aggregate to that version
// and change the root hash. otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_copy_aggregate_source_open()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return true;
    }

    const auto closed = root_last_hashed_index();
    if (!closed)
    {
        return true;
    }

    const auto& source = std::get<wizard::copy_aggregate_data_t>(command.data).source;
    const auto* aggregate = language_part_.aggregates().try_by_name(source);
    if (!aggregate)
    {
        return true;
    }

    for (const auto& declaration : language_part_.versions().declarations_in_order(aggregate->vd))
    {
        if (declaration.status == version_status_et::DELETED || declaration.index > *closed)
        {
            continue;
        }

        using enum exception::details::validation_error_code_et;

        fail(WIZARD_COPY_AGGREGATE_SOURCE_IN_CLOSED_VERSION, std::format(
            "{}: {}: {}",
            wizard::tags::copy_aggregate::command,
            wizard::tags::copy_aggregate::source_in_closed_version,
            source
        ), {
            .path = command.chunk_uri,
            .aggregate = source,
            .version = declaration.index
        }
        );
    }

    return true;
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
