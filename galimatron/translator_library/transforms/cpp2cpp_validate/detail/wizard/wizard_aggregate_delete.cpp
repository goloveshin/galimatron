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

// delete aggregate runs in the chunk context, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_context_chunk()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_DELETE_AGGREGATE_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

// delete aggregate needs the root aggregate, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_root_aggregate_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_DELETE_AGGREGATE_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

// names in one delete aggregate command differ, ignoring case,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_names_unique()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
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

    fail(WIZARD_DELETE_AGGREGATE_NAMES_MUST_BE_UNIQUE, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::names_must_be_unique,
        boost::join(duplicated, ",")
    ), where
    );
}

// each listed aggregate exists, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_aggregates_exist()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    const auto& names = std::get<wizard::create_aggregate_data_t>(command.data).names;
    strings_t missing;
    for (const auto& name : names)
    {
        if (try_aggregate(name) || name_already_listed(missing, name))
        {
            continue;
        }
        missing.push_back(name);
    }

    if (missing.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (missing.size() == 1)
    {
        where.aggregate = missing.front();
    }

    fail(WIZARD_DELETE_AGGREGATE_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::aggregate_must_exist,
        boost::join(missing, ",")
    ), where
    );
}

// the root aggregate is not a delete target, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_not_root()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    const auto& names = std::get<wizard::create_aggregate_data_t>(command.data).names;
    strings_t roots;
    for (const auto& name : names)
    {
        const auto* record = try_aggregate(name);
        if (!record || !record->is_root || name_already_listed(roots, name))
        {
            continue;
        }
        roots.push_back(name);
    }

    if (roots.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (roots.size() == 1)
    {
        where.aggregate = roots.front();
    }

    fail(WIZARD_DELETE_AGGREGATE_CANNOT_DELETE_ROOT_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::cannot_delete_root_aggregate,
        boost::join(roots, ",")
    ), where
    );
}

// the last version of each listed aggregate is live,
// unless every version of that aggregate is already deleted.
// a fully deleted aggregate is removed as a chunk and is not an error here.
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_last_version_live()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    if (std::get<wizard::create_aggregate_data_t>(command.data).physically)
    {
        return true;
    }

    const auto& names = std::get<wizard::create_aggregate_data_t>(command.data).names;
    strings_t absent;
    strings_t deleted;
    for (const auto& name : names)
    {
        const auto* record = try_aggregate(name);
        if (!record || record->is_root)
        {
            continue;
        }

        const auto& declarations = record->declarations;
        if (declarations.empty())
        {
            if (!name_already_listed(absent, name))
            {
                absent.push_back(name);
            }
            continue;
        }

        bool fully_deleted = true;
        for (const auto& declaration : declarations)
        {
            if (declaration_status(*record, declaration.index) != version_status_et::DELETED)
            {
                fully_deleted = false;
                break;
            }
        }
        if (fully_deleted)
        {
            continue;
        }

        if (declaration_status(*record, declarations.back().index) == version_status_et::DELETED && !name_already_listed(deleted, name))
        {
            deleted.push_back(name);
        }
    }

    using enum exception::details::validation_error_code_et;

    if (!absent.empty())
    {
        exception::error_where_t where{
            .path = command.chunk_uri
        };
        if (absent.size() == 1)
        {
            where.aggregate = absent.front();
        }

        fail(WIZARD_DELETE_AGGREGATE_LAST_VERSION_MUST_EXIST, std::format(
            "{}: {}: {}",
            wizard::tags::delete_aggregate::command,
            wizard::tags::delete_aggregate::last_version_must_exist,
            boost::join(absent, ",")
        ), where
        );
    }

    if (deleted.empty())
    {
        return true;
    }

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (deleted.size() == 1)
    {
        where.aggregate = deleted.front();
        const auto* record = try_aggregate(deleted.front());
        const auto& declarations = record->declarations;
        where.version = declarations.back().index;
    }

    fail(WIZARD_DELETE_AGGREGATE_LAST_VERSION_IS_ALREADY_DELETED, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::last_version_is_already_deleted,
        boost::join(deleted, ",")
    ), where
    );
}

// the last version is open. a root version that stores a hash is closed, and so is every version up to that index.
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_last_version_open()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    if (!language_part_.aggregates().has_root())
    {
        return true;
    }

    if (std::get<wizard::create_aggregate_data_t>(command.data).physically)
    {
        return true;
    }

    const auto closed = root_last_hashed_index();
    if (!closed)
    {
        return true;
    }

    const auto& names = std::get<wizard::create_aggregate_data_t>(command.data).names;
    strings_t closed_names;
    index_ot version;
    for (const auto& name : names)
    {
        const auto* record = try_aggregate(name);
        if (!record || record->is_root)
        {
            continue;
        }

        const auto& declarations = record->declarations;
        if (declarations.empty() || declaration_status(*record, declarations.back().index) == version_status_et::DELETED)
        {
            continue;
        }

        if (declarations.back().index > *closed)
        {
            continue;
        }

        if (!name_already_listed(closed_names, name))
        {
            closed_names.push_back(name);
            version = declarations.back().index;
        }
    }

    if (closed_names.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (closed_names.size() == 1)
    {
        where.aggregate = closed_names.front();
        where.version = version;
    }

    fail(WIZARD_DELETE_AGGREGATE_CANNOT_DELETE_CLOSED_VERSION, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::cannot_delete_closed_version,
        boost::join(closed_names, ",")
    ), where
    );
}

// the last live version has a definition to receive delete;, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_version_definition_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    if (std::get<wizard::create_aggregate_data_t>(command.data).physically)
    {
        return true;
    }

    const auto& names = std::get<wizard::create_aggregate_data_t>(command.data).names;
    strings_t missing;
    index_ot version;
    for (const auto& name : names)
    {
        const auto* record = try_aggregate(name);
        if (!record || record->is_root)
        {
            continue;
        }

        const auto& declarations = record->declarations;
        if (declarations.empty() || declaration_status(*record, declarations.back().index) == version_status_et::DELETED)
        {
            continue;
        }

        if (language_part_.versions().try_definition(record->vd, declarations.back().index))
        {
            continue;
        }

        if (!name_already_listed(missing, name))
        {
            missing.push_back(name);
            version = declarations.back().index;
        }
    }

    if (missing.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (missing.size() == 1)
    {
        where.aggregate = missing.front();
        where.version = version;
    }

    fail(WIZARD_DELETE_AGGREGATE_VERSION_DEFINITION_MUST_EXIST, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::version_definition_must_exist,
        boost::join(missing, ",")
    ), where
    );
}

// physically removes the chunk. an aggregate still present in a closed version is refused,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_physically_closed()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    if (!data.physically || !language_part_.aggregates().has_root())
    {
        return true;
    }

    const auto closed = root_last_hashed_index();
    if (!closed)
    {
        return true;
    }

    strings_t closed_names;
    index_ot version;
    for (const auto& name : data.names)
    {
        const auto* record = try_aggregate(name);
        if (!record || record->is_root || name_already_listed(closed_names, name))
        {
            continue;
        }

        for (const auto& declaration : record->declarations)
        {
            if (declaration_status(*record, declaration.index) == version_status_et::DELETED || declaration.index > *closed)
            {
                continue;
            }

            closed_names.push_back(name);
            version = declaration.index;
            break;
        }
    }

    if (closed_names.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (closed_names.size() == 1)
    {
        where.aggregate = closed_names.front();
        where.version = version;
    }

    fail(WIZARD_DELETE_AGGREGATE_CANNOT_DELETE_CLOSED_VERSION, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::cannot_delete_closed_version,
        boost::join(closed_names, ",")
    ), where
    );
}

// a version definition under the root whose ordering names this aggregate
//
bool ordering_names_aggregate(
    const holder::language_part_t& language,
    const name_t& name
)
{
    if (!language.aggregates().has_root())
    {
        return false;
    }
    return language.ordering_names(name);
}

// physically removes the chunk. a version whose ordering names the aggregate is refused,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_delete_aggregate_physically_not_ordered()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return true;
    }

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    if (!data.physically)
    {
        return true;
    }

    strings_t named;
    for (const auto& name : data.names)
    {
        if (!ordering_names_aggregate(language_part_, name) || name_already_listed(named, name))
        {
            continue;
        }

        named.push_back(name);
    }

    if (named.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (named.size() == 1)
    {
        where.aggregate = named.front();
    }

    fail(WIZARD_DELETE_AGGREGATE_ORDERING_NAMES_AGGREGATE, std::format(
        "{}: {}: {}",
        wizard::tags::delete_aggregate::command,
        wizard::tags::delete_aggregate::ordering_names_the_aggregate,
        boost::join(named, ",")
    ), where
    );
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
