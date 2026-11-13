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

// versions whose stored hash create hash force or delete hash force does not check.
// empty when the command is absent. no version means the last version
//
indices_t cpp2cpp_validate_t::hash_force_indexes() const
{
    indices_t forced;
    if (language_part_.wizard_statements().empty())
    {
        return forced;
    }

    index_ot last;
    if (language_part_.aggregates().has_root())
    {
        const auto& declarations = root_versions().declarations;
        if (!declarations.empty())
        {
            last = declarations.back().index;
        }
    }

    const auto consider = [&](const wizard::command_et type, const wizard::command_data_t& data)
    {
        if (type != wizard::command_et::CREATE_HASH && type != wizard::command_et::DELETE_HASH)
        {
            return;
        }

        const auto* hash = std::get_if<wizard::hash_data_t>(&data);
        if (!hash || !hash->force)
        {
            return;
        }

        if (hash->version_indexes.empty())
        {
            if (last && !index_is_listed(forced, *last))
            {
                forced.push_back(*last);
            }
            return;
        }

        for (const auto index : hash->version_indexes)
        {
            if (!index_is_listed(forced, index))
            {
                forced.push_back(index);
            }
        }
    };

    const auto& command = wizard_statement(language_part_);
    if (command.type == wizard::command_et::BATCH)
    {
        for (const auto& item : command.commands)
        {
            consider(item.type, item.data);
        }
        return forced;
    }

    consider(command.type, command.data);
    return forced;
}

// the stored hash of a version declaration matches the one computed the same way as wizard create hash.
// a deleted declaration is left out. the root hash sorts the content hashes, so only the content is part of it.
// checked per version and against the root hash. no hash — the declaration is not checked.
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator,
// validation_error_code_et::HASH_DOES_NOT_MATCH.
// the message names the aggregates of each differing version, then a version that differs only in the root.
// a version that already names an aggregate does not name the root
//
bool cpp2cpp_validate_t::is_version_hashes_match()
{
    struct row_t
    {
        bool is_root = false;
        std::string name;
        std::string content_hash;
        string_ot stored;
    };

    const auto& graph = language_part_.graph();
    const auto forced = hash_force_indexes();
    strings_t aggregate_places;
    strings_t root_places;

    for (const auto& declaration : root_versions().declarations)
    {
        const auto index = declaration.index;
        if (index_is_listed(forced, index))
        {
            continue;
        }
        std::vector<row_t> rows;

        for (const auto& aggregate : aggregate_versions_)
        {
            const auto stored_declaration = language_part_.versions().try_stored(aggregate.vd, index);
            if (!stored_declaration || !stored_declaration->has_declaration || is_deleted(stored_declaration->status))
            {
                continue;
            }

            rows.push_back({
                .is_root = aggregate.is_root,
                .name = aggregate.name,
                .content_hash = content_hash::important_tags_hash(
                    graph,
                    language_part_.versions().try_definition(aggregate.vd, index)
                ),
                .stored = stored_declaration->hash
            });
        }

        const bool any_stored = std::ranges::any_of(rows, [](const row_t& row)
        {
            return row.stored.has_value();
        });
        if (!any_stored)
        {
            continue;
        }

        std::vector<std::string> hashes;
        for (const auto& row : rows)
        {
            hashes.push_back(row.content_hash);
        }
        const auto root_hash = content_hash::hash_of_hashes(hashes);

        strings_t aggregate_names;
        bool root_differs = false;
        std::string root_name;
        for (const auto& row : rows)
        {
            if (!row.stored)
            {
                continue;
            }

            const auto& expected = row.is_root ? root_hash : row.content_hash;
            if (*row.stored == expected)
            {
                continue;
            }

            if (row.is_root)
            {
                root_differs = true;
                root_name = row.name;
                continue;
            }

            aggregate_names.push_back(row.name);
        }

        if (!aggregate_names.empty())
        {
            aggregate_places.push_back(std::format(
                "version {}: {}",
                index,
                boost::join(aggregate_names, ", ")
            ));
            continue;
        }

        if (root_differs)
        {
            root_places.push_back(std::format(
                "version {}: {}",
                index,
                root_name
            ));
        }
    }

    aggregate_places.insert(aggregate_places.end(), root_places.begin(), root_places.end());
    if (aggregate_places.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(HASH_DOES_NOT_MATCH, std::format(
        "hash does not match: {}",
        boost::join(aggregate_places, "; ")
    ));
}

// a version that stores a hash stores it on the root and on every aggregate that is not deleted.
// a deleted declaration stays without a hash.
// a version with no hashes is not checked.
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
// and every declaration where the hash is absent
//
bool cpp2cpp_validate_t::is_version_hashes_present()
{
    struct presence_t
    {
        bool is_root = false;
        index_t aggregate_index = 0;
        std::string name;
        bool stored = false;
    };

    const auto& graph = language_part_.graph();
    const auto forced = hash_force_indexes();
    strings_t places;

    for (const auto& declaration : root_versions().declarations)
    {
        const auto index = declaration.index;
        if (index_is_listed(forced, index))
        {
            continue;
        }
        std::vector<presence_t> rows;

        for (const auto& aggregate : aggregate_versions_)
        {
            const auto stored_declaration = language_part_.versions().try_stored(aggregate.vd, index);
            if (!stored_declaration || !stored_declaration->has_declaration || is_deleted(stored_declaration->status))
            {
                continue;
            }

            index_t aggregate_index = 0;
            if (!aggregate.is_root)
            {
                aggregate_index = std::get<state::aggregate_t>(graph[aggregate.vd].entity).index;
            }

            rows.push_back({
                .is_root = aggregate.is_root,
                .aggregate_index = aggregate_index,
                .name = aggregate.name,
                .stored = stored_declaration->hash.has_value()
            });
        }

        const bool any_stored = std::ranges::any_of(rows, &presence_t::stored);
        if (!any_stored)
        {
            continue;
        }

        rows |= ranges::actions::sort([](const presence_t& lhs, const presence_t& rhs)
        {
            if (lhs.is_root != rhs.is_root)
            {
                return lhs.is_root;
            }
            return lhs.aggregate_index < rhs.aggregate_index;
        });

        for (const auto& row : rows)
        {
            if (row.stored)
            {
                continue;
            }
            places.push_back(std::format("{} version {}", row.name, index));
        }
    }

    if (places.empty())
    {
        return true;
    }

    using enum exception::details::validation_error_code_et;

    fail(HASH_MUST_BE_ON_EVERY_AGGREGATE, std::format(
        "hash is absent: {}",
        boost::join(places, ", ")
    ));
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
