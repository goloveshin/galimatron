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

// the root aggregate is exactly one, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
// none — validation_error_code_et::ROOT_AGGREGATE_MISSED
// more than one — validation_error_code_et::ROOT_AGGREGATE_MUST_BE_ONE_ONLY
//
bool cpp2cpp_validate_t::is_root_aggregate_single()
{
    if (const auto cached = memo_.get_is_root_aggregate_single())
    {
        return *cached;
    }

    const auto root_count = std::ranges::count(aggregate_versions_, true, &aggregate_versions_t::is_root);
    if (root_count == 1)
    {
        memo_.store_is_root_aggregate_single(true);
        return true;
    }

    using enum exception::details::validation_error_code_et;

    if (root_count == 0)
    {
        fail(ROOT_AGGREGATE_MISSED, "root aggregate is absent");
    }

    fail(ROOT_AGGREGATE_MUST_BE_ONE_ONLY, std::format(
        "root aggregate is not single: files has root aggregate: {}",
        join_root_chunk_paths(false)
    ));
}

// root aggregate version declarations run from 0 without gaps, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_root_aggregate_version_declarations_from_zero_continuous()
{
    if (const auto cached = memo_.get_is_root_aggregate_version_declarations_from_zero_continuous())
    {
        return *cached;
    }

    const auto indices = version_declaration_indices();

    if (!indices.empty())
    {
        const auto listed = [&]
        {
            return join_indices(indices);
        };

        using enum exception::details::validation_error_code_et;

        if (indices.front() != 0)
        {
            fail(ROOT_AGGREGATE_VERSION_DECLARATIONS_MUST_START_WITH_ZERO, std::format("root aggregate version declarations do not start with 0: {}", listed()));
        }

        for (index_t position = 1; position < indices.size(); ++position)
        {
            if (indices[position] != indices[position - 1] + 1)
            {
                fail(ROOT_AGGREGATE_VERSION_DECLARATIONS_MUST_BE_CONTINUOUS, std::format("root aggregate version declarations are not continuous: {}", listed()));
            }
        }
    }

    memo_.store_is_root_aggregate_version_declarations_from_zero_continuous(true);
    return true;
}

// root aggregate version definition indices repeat the live declaration indices, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_root_aggregate_version_definitions_repeat_declarations()
{
    if (const auto cached = memo_.get_is_root_aggregate_version_definitions_repeat_declarations())
    {
        return *cached;
    }

    const auto declarations = root_versions().live_indices;
    const auto definitions = version_definition_indices();

    if (definitions != declarations)
    {
        using enum exception::details::validation_error_code_et;

        fail(ROOT_AGGREGATE_VERSION_DEFINITIONS_MUST_REPEAT_DECLARATIONS, std::format(
            "root aggregate version definitions do not repeat declarations: declarations {}, definitions {}",
            join_indices(declarations),
            join_indices(definitions)
        ));
    }

    using enum exception::details::validation_error_code_et;

    const auto root = language_part_.aggregates().root();
    for (const auto& body : language_part_.versions().bodies_in_order(root.vd))
    {
        const auto& entity = language_part_.graph()[body.vd].entity;
        const auto* definition = std::get_if<root_aggregate_version_definition_t>(&entity);
        if (!definition)
        {
            continue;
        }
        if (definition->mode != tag_mode_et::LOCAL && definition->mode != tag_mode_et::OWN)
        {
            continue;
        }

        const auto label = definition->index
            ? std::to_string(*definition->index)
            : definition->owner
                ? std::format("before {}", *definition->owner)
                : std::string{ "without an index" };
        const auto word = definition->mode == tag_mode_et::LOCAL ? "local" : "own";
        fail(
            definition->mode == tag_mode_et::LOCAL
                ? ROOT_AGGREGATE_VERSION_CANNOT_BE_LOCAL
                : ROOT_AGGREGATE_VERSION_CANNOT_BE_OWN,
            std::format("root aggregate version {} cannot be {}", label, word),
            {
                .aggregate = root.name,
                .version = definition->index
            }
        );
    }

    memo_.store_is_root_aggregate_version_definitions_repeat_declarations(true);
    return true;
}

// each aggregate's version declarations repeat the root declarations, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_aggregate_version_declarations_match_root()
{
    const auto root_indices = version_declaration_indices();

    for (const auto& aggregate : aggregate_versions_)
    {
        if (aggregate.is_root)
        {
            continue;
        }

        const auto indices = declaration_indices(aggregate);
        if (indices == root_indices)
        {
            continue;
        }

        using enum exception::details::validation_error_code_et;

        fail(AGGREGATE_VERSION_DECLARATIONS_MUST_MATCH_ROOT, std::format(
            "aggregate '{}' version declarations do not match root aggregate: root {}, declarations {}",
            aggregate.name,
            join_indices(root_indices),
            join_indices(indices)
        ), {
            .aggregate = aggregate.name
        }
        );
    }

    return true;
}

// deleted is a prefix, a suffix, or an interior span owned by the next live version's anonymous body, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_aggregate_version_declarations_deleted_continuous()
{
    for (const auto& aggregate : aggregate_versions_)
    {
        const auto& declarations = aggregate.declarations;

        bool illegal = false;
        for (const auto& declaration : declarations)
        {
            if (!is_deleted(declaration_status(aggregate, declaration.index)))
            {
                continue;
            }

            const bool live_before = std::ranges::any_of(declarations, [&](const auto& other)
            {
                return other.index < declaration.index && !is_deleted(declaration_status(aggregate, other.index));
            });
            index_ot owner;
            for (const auto& other : declarations)
            {
                if (other.index > declaration.index && !is_deleted(declaration_status(aggregate, other.index)))
                {
                    owner = other.index;
                    break;
                }
            }
            if (!live_before || !owner)
            {
                continue;
            }

            const auto covered = std::ranges::any_of(
                language_part_.versions().anonymous_owned_by(aggregate.vd, *owner),
                [&](const auto& anonymous)
                {
                    return !language_part_.is_only_version_delete(anonymous.vd_definition);
                }
            );
            if (!covered)
            {
                illegal = true;
            }
        }

        if (!illegal)
        {
            continue;
        }

        using enum exception::details::validation_error_code_et;

        fail(AGGREGATE_VERSION_DECLARATIONS_DELETED_MUST_BE_CONTINUOUS, std::format(
            "aggregate '{}' version declarations: deleted status is not continuous: {}",
            aggregate.name,
            join_declaration_statuses(aggregate)
        ), {
            .aggregate = aggregate.name
        }
        );
    }

    return true;
}

// a definition of a deleted version is forbidden, except a body of a single delete;, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_aggregate_version_definitions_not_deleted()
{
    for (const auto& aggregate : aggregate_versions_)
    {
        const auto& definitions = aggregate.definition_indices;

        indices_t deleted_definitions;
        for (const auto index : definitions)
        {
            if (ranges::find(aggregate.deleted_indices, index) == aggregate.deleted_indices.end())
            {
                continue;
            }
            if (definition_is_only_version_delete(language_part_, aggregate.vd, index))
            {
                continue;
            }
            deleted_definitions.push_back(index);
        }

        if (deleted_definitions.empty())
        {
            continue;
        }

        using enum exception::details::validation_error_code_et;

        fail(AGGREGATE_VERSION_DEFINITIONS_MUST_NOT_BE_DELETED, std::format(
            "aggregate '{}' version definition must not be deleted: {}",
            aggregate.name,
            join_indices(deleted_definitions)
        ), {
            .aggregate = aggregate.name
        }
        );
    }

    return true;
}

// aggregate version definitions repeat the declarations without deleted, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_aggregate_version_definitions_repeat_declarations()
{
    for (const auto& aggregate : aggregate_versions_)
    {
        if (aggregate.is_root)
        {
            continue;
        }

        const auto& definitions = aggregate.definition_indices;
        const auto& live_declarations = aggregate.live_indices;

        // an illegal deleted definition is reported by is_aggregate_version_definitions_not_deleted.
        // the checks run in parallel, so we do not throw the same error here.
        // delete; of a deleted version is not included in the comparison with live declarations
        indices_t comparable;
        bool illegal_deleted = false;
        for (const auto index : definitions)
        {
            const auto deleted = ranges::find(aggregate.deleted_indices, index) != aggregate.deleted_indices.end();
            if (!deleted)
            {
                comparable.push_back(index);
                continue;
            }
            if (definition_is_only_version_delete(language_part_, aggregate.vd, index))
            {
                continue;
            }
            illegal_deleted = true;
        }
        if (illegal_deleted)
        {
            continue;
        }

        if (comparable == live_declarations)
        {
            continue;
        }

        using enum exception::details::validation_error_code_et;

        fail(AGGREGATE_VERSION_DEFINITIONS_MUST_REPEAT_DECLARATIONS, std::format(
            "aggregate '{}' version definitions do not repeat declarations: declarations {}, definitions {}",
            aggregate.name,
            join_indices(live_declarations),
            join_indices(comparable)
        ), {
            .aggregate = aggregate.name
        }
        );
    }

    return true;
}

bool cpp2cpp_validate_t::is_anonymous_versions_owned()
{
    for (const auto& aggregate : aggregate_versions_)
    {
        const auto bodies = language_part_.versions().bodies_in_order(aggregate.vd);

        for (index_t position = 0; position < bodies.size(); ++position)
        {
            const auto& body = bodies[position];
            if (body.index)
            {
                continue;
            }

            const auto owner = body.owner;
            index_ot next;
            for (index_t later = position + 1; later < bodies.size(); ++later)
            {
                if (bodies[later].index)
                {
                    next = bodies[later].index;
                    break;
                }
            }

            using enum exception::details::validation_error_code_et;

            if (!owner || !next)
            {
                if (owner)
                {
                    fail(ANONYMOUS_VERSION_MUST_HAVE_OWNER, std::format(
                        "aggregate '{}' version before {} must have an owner",
                        aggregate.name,
                        *owner
                    ), {
                        .aggregate = aggregate.name,
                        .version = *owner
                    }
                    );
                }
                fail(ANONYMOUS_VERSION_MUST_HAVE_OWNER, std::format(
                    "aggregate '{}' anonymous version must have an owner",
                    aggregate.name
                ), {
                    .aggregate = aggregate.name
                }
                );
            }

            if (*owner != *next)
            {
                fail(ANONYMOUS_VERSION_MUST_PRECEDE_OWNER, std::format(
                    "aggregate '{}' version before {} must be immediately before that version",
                    aggregate.name,
                    *owner
                ), {
                    .aggregate = aggregate.name,
                    .version = *owner
                }
                );
            }

            const auto stored = language_part_.versions().try_stored(aggregate.vd, *owner);
            if (!stored || !stored->has_declaration)
            {
                fail(ANONYMOUS_VERSION_MUST_HAVE_OWNER, std::format(
                    "aggregate '{}' version before {} must have an owner",
                    aggregate.name,
                    *owner
                ), {
                    .aggregate = aggregate.name,
                    .version = *owner
                }
                );
            }

            if (!is_deleted(stored->status))
            {
                continue;
            }

            if (language_part_.is_only_version_delete(body.vd))
            {
                continue;
            }

            fail(ANONYMOUS_VERSION_MUST_BE_DELETED, std::format(
                "aggregate '{}' anonymous version must be deleted: owner {}",
                aggregate.name,
                *owner
            ), {
                .aggregate = aggregate.name,
                .version = *owner
            }
            );
        }
    }

    return true;
}

// aggregate names are unique, including the root.
// no single root aggregate is left to is_root_aggregate_single.
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_aggregate_names_unique()
{
    const auto root_count = std::ranges::count(aggregate_versions_, true, &aggregate_versions_t::is_root);
    if (root_count != 1)
    {
        return true;
    }

    auto names =
        aggregate_versions_
        | ranges::views::transform([](const aggregate_versions_t& aggregate)
        {
            return aggregate.name;
        })
        | ranges::to<strings_t>()
        | ranges::actions::sort
        ;

    strings_t duplicated;
    for (index_t position = 1; position < names.size(); ++position)
    {
        if (names[position] != names[position - 1])
        {
            continue;
        }
        if (!duplicated.empty() && duplicated.back() == names[position])
        {
            continue;
        }
        duplicated.push_back(names[position]);
    }

    if (duplicated.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(AGGREGATE_NAMES_MUST_BE_UNIQUE, std::format(
        "aggregate names are not unique: {}",
        boost::join(duplicated, ",")
    ));
}

// non-root aggregate indices are 0 .. n-1 without gaps or duplicates,
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
//
bool cpp2cpp_validate_t::is_aggregate_indices_unique_continuous()
{
    const auto& graph = language_part_.graph();
    auto indices =
        aggregate_versions_
        | ranges::views::filter([](const aggregate_versions_t& aggregate)
        {
            return !aggregate.is_root;
        })
        | ranges::views::transform([&](const aggregate_versions_t& aggregate)
        {
            return std::get<state::aggregate_t>(graph[aggregate.vd].entity).index;
        })
        | ranges::to<indices_t>()
        | ranges::actions::sort
        ;

    for (index_t position = 0; position < indices.size(); ++position)
    {
        if (indices[position] == position)
        {
            continue;
        }

        using enum exception::details::validation_error_code_et;

        fail(AGGREGATE_INDICES_MUST_BE_UNIQUE_CONTINUOUS, std::format(
            "aggregate indices are not unique and continuous from 0: {}",
            join_indices(indices)
        ));
    }

    return true;
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
