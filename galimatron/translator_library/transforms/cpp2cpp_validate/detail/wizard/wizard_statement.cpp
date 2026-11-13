// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/wizard/tags.hpp"
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/exception_augment.hpp"

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

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

namespace
{

struct version_statement_problem_t
{
    exception::details::validation_error_code_et code;
    std::string explain;
    std::string what;
    exception::error_where_t where;
};

std::string_view version_statement_command(const wizard::command_et type)
{
    using enum wizard::command_et;

    switch (type)
    {
        case CREATE_TAG:
            return wizard::tags::create_tag::command;
        case DELETE_TAG:
            return wizard::tags::delete_tag::command;
        case CREATE_LABEL:
            return wizard::tags::create_label::command;
        case CREATE_ORDERING:
            return wizard::tags::create_ordering::command;
        default:
            break;
    }
    return {};
}

name_ot version_statement_aggregate_name(const wizard_statement_t& command)
{
    using enum wizard::command_et;

    switch (command.type)
    {
        case CREATE_TAG:
            return std::get<wizard::create_tag_data_t>(command.data).aggregate;
        case DELETE_TAG:
            return std::get<wizard::delete_tag_data_t>(command.data).aggregate;
        case CREATE_LABEL:
            return std::get<wizard::label_data_t>(command.data).aggregate;
        case CREATE_ORDERING:
            return std::get<wizard::ordering_data_t>(command.data).aggregate;
        default:
            break;
    }
    return {};
}

// the first reason the command cannot write into the open version. empty when it can
//
struct version_statement_target_t
{
    bool found = false;
    bool is_root = false;
    name_t name;
    vertex_descriptor_t vd = vertex_descriptor_not_present;
    bool has_declaration = false;
    index_t last_index = 0;
    version_status_oet last_status;
};
std::vector<version_statement_problem_t> version_statement_problems(
    const holder::language_part_t& language,
    const version_statement_target_t& aggregate,
    const bool has_root,
    const index_ot closed,
    const wizard::command_et type,
    const wizard::context_et context,
    const path_t& chunk_uri,
    const name_ot& aggregate_name
)
{
    using enum exception::details::validation_error_code_et;
    namespace phrase = wizard::tags::version_statement;

    const auto command = version_statement_command(type);
    const bool ordering = type == wizard::command_et::CREATE_ORDERING;
    const bool context_ok = context == wizard::context_et::CHUNK;

    exception::error_where_t where{
        .path = chunk_uri
    };

    const auto one = [](
        const exception::details::validation_error_code_et code,
        std::string explain,
        std::string what,
        const exception::error_where_t& problem_where
    )
    {
        return std::vector<version_statement_problem_t>{{
            .code = code,
            .explain = std::move(explain),
            .what = std::move(what),
            .where = problem_where
        }};
    };

    if (!context_ok)
    {
        return one(
            WIZARD_VERSION_STATEMENT_CONTEXT_MUST_BE_CHUNK,
            std::format("{}: {}", phrase::context_must_be_chunk, to_string(context)),
            std::format("{} {}: {}", command, phrase::context_must_be_chunk, to_string(context)),
            where
        );
    }

    if (!aggregate_name)
    {
        return one(
            WIZARD_VERSION_STATEMENT_AGGREGATE_MUST_BE_NAMED,
            std::string(phrase::aggregate_must_be_named),
            std::format("{}: {}", command, phrase::aggregate_must_be_named),
            where
        );
    }

    if (!has_root)
    {
        return one(
            WIZARD_VERSION_STATEMENT_ROOT_AGGREGATE_MUST_EXIST,
            std::string(wizard::tags::delete_aggregate::root_aggregate_must_exist),
            std::format("{}: {}", command, wizard::tags::delete_aggregate::root_aggregate_must_exist),
            where
        );
    }

    if (!aggregate.found)
    {
        where.aggregate = *aggregate_name;
        return one(
            WIZARD_VERSION_STATEMENT_AGGREGATE_MUST_EXIST,
            std::format("{}: {}", wizard::tags::delete_aggregate::aggregate_must_exist, *aggregate_name),
            std::format("{}: {}: {}", command, wizard::tags::delete_aggregate::aggregate_must_exist, *aggregate_name),
            where
        );
    }

    where.aggregate = aggregate.name;
    if (ordering && !aggregate.is_root)
    {
        return one(
            WIZARD_VERSION_STATEMENT_AGGREGATE_MUST_BE_ROOT,
            std::format("{}: {}", phrase::aggregate_must_be_root, aggregate.name),
            std::format("{}: {}: {}", command, phrase::aggregate_must_be_root, aggregate.name),
            where
        );
    }

    if (!aggregate.has_declaration)
    {
        return one(
            WIZARD_VERSION_STATEMENT_LAST_VERSION_MUST_EXIST,
            std::format("{}: {}", wizard::tags::delete_aggregate::last_version_must_exist, aggregate.name),
            std::format("{}: {}: {}", command, wizard::tags::delete_aggregate::last_version_must_exist, aggregate.name),
            where
        );
    }

    where.version = aggregate.last_index;
    if (aggregate.last_status == version_status_et::DELETED)
    {
        return one(
            WIZARD_VERSION_STATEMENT_VERSION_IS_DELETED,
            std::format("{}: {}", phrase::version_is_deleted, aggregate.name),
            std::format("{}: {}: {}", command, phrase::version_is_deleted, aggregate.name),
            where
        );
    }

    if (closed && aggregate.last_index <= *closed)
    {
        return one(
            WIZARD_VERSION_STATEMENT_VERSION_MUST_BE_OPEN,
            std::format("{}: {}", phrase::version_must_be_open, aggregate.name),
            std::format("{}: {}: {}", command, phrase::version_must_be_open, aggregate.name),
            where
        );
    }

    if (!language.versions().try_definition(aggregate.vd, aggregate.last_index))
    {
        return one(
            WIZARD_VERSION_STATEMENT_DEFINITION_MUST_EXIST,
            std::format("{}: {}", wizard::tags::delete_aggregate::version_definition_must_exist, aggregate.name),
            std::format("{}: {}: {}", command, wizard::tags::delete_aggregate::version_definition_must_exist, aggregate.name),
            where
        );
    }

    return {};
}

}// namespace

bool cpp2cpp_validate_t::is_wizard_batch_context_chunk()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::BATCH)
    {
        return true;
    }

    if (command.context == wizard::context_et::CHUNK)
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_BATCH_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::batch::command,
        wizard::tags::batch::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_batch_command_create()
{
    const auto& command = wizard_statement(language_part_);
    switch (command.type)
    {
        case wizard::command_et::CREATE_ROOT_AGGREGATE:
        case wizard::command_et::CREATE_AGGREGATE:
        case wizard::command_et::CREATE_VERSION:
        case wizard::command_et::DELETE_VERSION:
        case wizard::command_et::COLLAPSE_VERSION:
        case wizard::command_et::CREATE_LABEL:
        case wizard::command_et::CREATE_TAG:
        case wizard::command_et::DELETE_TAG:
        case wizard::command_et::CREATE_ORDERING:
        case wizard::command_et::CREATE_HASH:
            return true;
        default:
            break;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_BATCH_COMMAND_MUST_BE_CREATE, std::format(
        "{} {}: wizard {}",
        wizard::tags::batch::command,
        wizard::tags::batch::command_must_be_allowed,
        to_string(command.type)
    ), {
        .path = command.chunk_uri
    }
    );
}

// create tag, delete tag, create label, and create ordering write into the open version,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_wizard_version_statement()
{
    const auto& command = wizard_statement(language_part_);
    switch (command.type)
    {
        case wizard::command_et::CREATE_LABEL:
        case wizard::command_et::CREATE_TAG:
        case wizard::command_et::DELETE_TAG:
        case wizard::command_et::CREATE_ORDERING:
            break;
        default:
            return true;
    }

    const auto aggregate_name = version_statement_aggregate_name(command);
    const auto* found = aggregate_name ? try_aggregate(*aggregate_name) : nullptr;
    version_statement_target_t target;
    if (found)
    {
        target.found = true;
        target.is_root = found->is_root;
        target.name = found->name;
        target.vd = found->vd;
        if (!found->declarations.empty())
        {
            target.has_declaration = true;
            target.last_index = found->declarations.back().index;
            target.last_status = declaration_status(*found, found->declarations.back().index);
        }
    }
    const auto problems = version_statement_problems(
        language_part_,
        target,
        root_is_present(),
        root_last_hashed_index(),
        command.type,
        command.context,
        command.chunk_uri,
        aggregate_name
    );
    if (problems.empty())
    {
        return true;
    }

    const auto& problem = problems.front();
    fail(problem.code, problem.what, problem.where);
}

bool cpp2cpp_validate_t::wizard_hash_context_is_chunk(const wizard::context_et context) const
{
    return context == wizard::context_et::CHUNK;
}

bool cpp2cpp_validate_t::is_wizard_create_hash_context()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_HASH)
    {
        return true;
    }

    if (wizard_hash_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_HASH_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::create_hash::command,
        wizard::tags::create_hash::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_delete_hash_context()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_HASH)
    {
        return true;
    }

    if (wizard_hash_context_is_chunk(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_DELETE_HASH_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::delete_hash::command,
        wizard::tags::delete_hash::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

indices_t cpp2cpp_validate_t::hash_version_indexes()
{
    const auto& command = wizard_statement(language_part_);
    const auto& requested = std::get<wizard::hash_data_t>(command.data).version_indexes;
    if (!requested.empty())
    {
        return requested;
    }

    if (!language_part_.aggregates().has_root())
    {
        return {};
    }

    const auto& declarations = root_versions().declarations;
    if (declarations.empty())
    {
        return {};
    }

    return { declarations.back().index };
}

bool cpp2cpp_validate_t::is_wizard_hash_version_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_HASH && command.type != wizard::command_et::DELETE_HASH)
    {
        return true;
    }

    const auto& requested = std::get<wizard::hash_data_t>(command.data).version_indexes;
    if (requested.empty())
    {
        if (!hash_version_indexes().empty())
        {
            return true;
        }

        using enum exception::details::validation_error_code_et;

        const auto is_create = command.type == wizard::command_et::CREATE_HASH;
        fail(
            is_create
                ? WIZARD_CREATE_HASH_VERSION_MUST_EXIST
                : WIZARD_DELETE_HASH_VERSION_MUST_EXIST,
            std::format(
                "{}: {}",
                is_create ? wizard::tags::create_hash::command : wizard::tags::delete_hash::command,
                is_create ? wizard::tags::create_hash::last_version_must_exist : wizard::tags::delete_hash::last_version_must_exist
            ),
            {
                .path = command.chunk_uri
            }
        );
    }

    indices_t missing;
    for (const auto index : requested)
    {
        const bool present = root_is_present() && declaration_at(root_versions(), index);
        if (!present && std::ranges::find(missing, index) == missing.end())
        {
            missing.push_back(index);
        }
    }

    if (missing.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    const auto is_create = command.type == wizard::command_et::CREATE_HASH;
    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (missing.size() == 1)
    {
        where.version = missing.front();
    }

    fail(
        is_create
            ? WIZARD_CREATE_HASH_VERSION_MUST_EXIST
            : WIZARD_DELETE_HASH_VERSION_MUST_EXIST,
        std::format(
            "{}: {}: {}",
            is_create ? wizard::tags::create_hash::command : wizard::tags::delete_hash::command,
            is_create ? wizard::tags::create_hash::version_must_exist : wizard::tags::delete_hash::version_must_exist,
            join_indices(missing)
        ),
        where
    );
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
