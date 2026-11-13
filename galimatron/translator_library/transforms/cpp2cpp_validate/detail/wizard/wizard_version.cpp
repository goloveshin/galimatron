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

bool cpp2cpp_validate_t::wizard_create_version_context_is_allowed(const wizard::context_et context) const
{
    return context == wizard::context_et::CHUNK;
}

bool cpp2cpp_validate_t::is_wizard_create_version_context()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_VERSION)
    {
        return true;
    }

    if (wizard_create_version_context_is_allowed(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_VERSION_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::create_version::command,
        wizard::tags::create_version::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_create_version_root_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::CREATE_VERSION)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_CREATE_VERSION_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::create_version::command,
        wizard::tags::create_version::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_delete_version_context()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_VERSION)
    {
        return true;
    }

    if (wizard_create_version_context_is_allowed(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_DELETE_VERSION_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::delete_version::command,
        wizard::tags::delete_version::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_delete_version_root_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_VERSION)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_DELETE_VERSION_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::delete_version::command,
        wizard::tags::delete_version::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_delete_version_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_VERSION)
    {
        return true;
    }

    if (!language_part_.aggregates().has_root())
    {
        return true;
    }

    const auto& requested = std::get<wizard::create_version_data_t>(command.data).version_indexes;
    const auto& declarations = root_versions().declarations;
    if (requested.empty())
    {
        if (!declarations.empty())
        {
            return true;
        }

        using enum exception::details::validation_error_code_et;

        fail(WIZARD_DELETE_VERSION_LAST_VERSION_MUST_EXIST, std::format(
            "{}: {}",
            wizard::tags::delete_version::command,
            wizard::tags::delete_version::last_version_must_exist
        ), {
            .path = command.chunk_uri
        }
        );
    }

    const auto declared =
        declarations
        | ranges::views::transform([](const auto& declaration){ return declaration.index; })
        | ranges::to<indices_t>()
        ;
    auto missing =
        requested
        | ranges::views::filter([&](const index_t index)
        {
            return !index_is_listed(declared, index);
        })
        | ranges::to<indices_t>()
        ;
    missing |= ranges::actions::sort | ranges::actions::unique;

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
        where.version = missing.front();
    }

    fail(
        WIZARD_DELETE_VERSION_VERSION_MUST_EXIST,
        std::format(
            "{}: {}: {}",
            wizard::tags::delete_version::command,
            wizard::tags::delete_version::version_must_exist,
            join_indices(missing)
        ),
        where
    );
}

bool cpp2cpp_validate_t::is_wizard_delete_version_open()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_VERSION)
    {
        return true;
    }

    if (!language_part_.aggregates().has_root())
    {
        return true;
    }

    const auto closed = root_last_hashed_index();
    if (!closed)
    {
        return true;
    }

    const auto indexes = delete_version_indexes();
    auto sealed =
        indexes
        | ranges::views::filter([&](const index_t index){ return index <= *closed; })
        | ranges::to<indices_t>()
        ;
    sealed |= ranges::actions::sort | ranges::actions::unique;

    if (sealed.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    exception::error_where_t where{
        .path = command.chunk_uri
    };
    if (sealed.size() == 1)
    {
        where.version = sealed.front();
    }

    fail(
        WIZARD_DELETE_VERSION_CANNOT_DELETE_CLOSED_VERSION,
        std::format(
            "{}: {}: {}",
            wizard::tags::delete_version::command,
            wizard::tags::delete_version::cannot_delete_closed_version,
            join_indices(sealed)
        ),
        where
    );
}

bool cpp2cpp_validate_t::is_wizard_delete_version_continuous()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::DELETE_VERSION)
    {
        return true;
    }

    if (!language_part_.aggregates().has_root())
    {
        return true;
    }

    const auto indexes = delete_version_indexes();
    if (indexes.empty())
    {
        return true;
    }

    const auto& declarations = root_versions().declarations;
    const auto remaining =
        declarations
        | ranges::views::transform([](const auto& declaration){ return declaration.index; })
        | ranges::views::filter([&](const index_t index)
        {
            return !index_is_listed(indexes, index);
        })
        | ranges::to<indices_t>()
        ;

    if (indexes_stay_continuous(remaining))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(
        WIZARD_DELETE_VERSION_MUST_STAY_CONTINUOUS,
        std::format(
            "{}: {}: {}",
            wizard::tags::delete_version::command,
            wizard::tags::delete_version::versions_must_stay_continuous,
            join_indices(remaining)
        ),
        {
            .path = command.chunk_uri
        }
    );
}

bool cpp2cpp_validate_t::is_wizard_collapse_version_context()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COLLAPSE_VERSION)
    {
        return true;
    }

    if (wizard_create_version_context_is_allowed(command.context))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COLLAPSE_VERSION_CONTEXT_MUST_BE_CHUNK, std::format(
        "{} {}: {}",
        wizard::tags::collapse_version::command,
        wizard::tags::collapse_version::context_must_be_chunk,
        to_string(command.context)
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_collapse_version_root_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COLLAPSE_VERSION)
    {
        return true;
    }

    if (wizard_create_aggregate_root_aggregate_is_present())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COLLAPSE_VERSION_ROOT_AGGREGATE_MUST_EXIST, std::format(
        "{}: {}",
        wizard::tags::collapse_version::command,
        wizard::tags::collapse_version::root_aggregate_must_exist
    ), {
        .path = command.chunk_uri
    }
    );
}

bool cpp2cpp_validate_t::is_wizard_collapse_version_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COLLAPSE_VERSION)
    {
        return true;
    }

    if (!language_part_.aggregates().has_root())
    {
        return true;
    }

    const auto& range = std::get<wizard::collapse_version_data_t>(command.data);
    if (range.first > range.last)
    {
        using enum exception::details::validation_error_code_et;

        fail(WIZARD_COLLAPSE_VERSION_RANGE_MUST_ASCEND, std::format(
            "{}: {}: {}-{}",
            wizard::tags::collapse_version::command,
            wizard::tags::collapse_version::range_must_ascend,
            range.first,
            range.last
        ), {
            .path = command.chunk_uri
        }
        );
    }

    const auto& declarations = root_versions().declarations;
    const auto declared =
        declarations
        | ranges::views::transform([](const auto& declaration){ return declaration.index; })
        | ranges::to<indices_t>()
        ;
    indices_t missing;
    for (index_t index = range.first; index <= range.last; ++index)
    {
        if (!index_is_listed(declared, index))
        {
            missing.push_back(index);
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
        where.version = missing.front();
    }

    fail(
        WIZARD_COLLAPSE_VERSION_VERSION_MUST_EXIST,
        std::format(
            "{}: {}: {}",
            wizard::tags::collapse_version::command,
            wizard::tags::collapse_version::version_must_exist,
            join_indices(missing)
        ),
        where
    );
}

bool cpp2cpp_validate_t::is_wizard_collapse_version_owner_present()
{
    const auto& command = wizard_statement(language_part_);
    if (command.type != wizard::command_et::COLLAPSE_VERSION)
    {
        return true;
    }

    if (!language_part_.aggregates().has_root())
    {
        return true;
    }

    const auto& range = std::get<wizard::collapse_version_data_t>(command.data);
    if (range.first > range.last)
    {
        return true;
    }

    const auto& declarations = root_versions().declarations;
    index_ot last_index;
    for (const auto& declaration : declarations)
    {
        if (!last_index || declaration.index > *last_index)
        {
            last_index = declaration.index;
        }
    }
    if (last_index && range.last == *last_index)
    {
        return true;
    }

    const auto owner = range.last + 1;
    const auto* declaration = declaration_at(root_versions(), owner);
    if (declaration && !is_deleted(declaration_status(root_versions(), owner)))
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(WIZARD_COLLAPSE_VERSION_OWNER_MUST_BE_PRESENT, std::format(
        "{}: {}: {}",
        wizard::tags::collapse_version::command,
        wizard::tags::collapse_version::owner_must_be_present,
        owner
    ), {
        .path = command.chunk_uri,
        .version = owner
    }
    );
}

indices_t cpp2cpp_validate_t::delete_version_indexes()
{
    const auto& command = wizard_statement(language_part_);
    const auto& requested = std::get<wizard::create_version_data_t>(command.data).version_indexes;
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

}// namespace galimatron::translator::transforms::cpp2cpp_validate
