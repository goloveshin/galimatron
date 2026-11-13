// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
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

cpp2cpp_validate_t::cpp2cpp_validate_t(
    const state::holder::language_part_t& language_part,
    validate_memo_t& memo
)
    : language_part_(language_part)
    , memo_(memo)
{
    for (const auto& aggregate : language_part_.aggregates())
    {
        const auto declarations = language_part_.versions().declarations_in_order(aggregate.vd);

        aggregate_versions_t row{
            .name = aggregate.name,
            .vd = aggregate.vd,
            .is_root = aggregate.is_root,
            .chunk_path = aggregate.chunk_path
        };

        for (const auto& declaration : declarations)
        {
            row.declarations.push_back({
                .index = declaration.index
            });

            if (is_deleted(declaration.status))
            {
                row.deleted_indices.push_back(declaration.index);
            }
            else
            {
                row.live_indices.push_back(declaration.index);
            }
        }

        row.definition_indices = language_part_.versions().definition_indices(aggregate.vd);
        if (aggregate.is_root)
        {
            row.last_hashed_index = aggregate.last_hashed;
            if (!root_vd_)
            {
                root_vd_ = aggregate.vd;
            }
        }
        aggregate_versions_.push_back(std::move(row));
    }
}

indices_t cpp2cpp_validate_t::declaration_indices(const aggregate_versions_t& aggregate) const
{
    auto declarations = ranges::subrange(aggregate.declarations.begin(), aggregate.declarations.end());
    return
        declarations
        | ranges::views::transform([](const aggregate_version_declaration_t& declaration)
        {
            return declaration.index;
        })
        | ranges::to<indices_t>()
        ;
}

const cpp2cpp_validate_t::aggregate_version_declaration_t* cpp2cpp_validate_t::declaration_at(
    const aggregate_versions_t& aggregate,
    const index_t index
) const
{
    const auto found = aggregate.declarations.try_find_by<by_declaration_index>(index);
    if (!found)
    {
        return nullptr;
    }
    return &found->get();
}

version_status_oet cpp2cpp_validate_t::declaration_status(
    const aggregate_versions_t& aggregate,
    const index_t index
) const
{
    return language_part_.versions().status(aggregate.vd, index);
}

const cpp2cpp_validate_t::aggregate_versions_t* cpp2cpp_validate_t::try_aggregate(const name_t& name) const
{
    for (const auto& aggregate : aggregate_versions_.equal_range_by<by_aggregate_name>(name))
    {
        return &aggregate;
    }
    return nullptr;
}

bool cpp2cpp_validate_t::root_is_present() const
{
    return root_vd_.has_value();
}

index_ot cpp2cpp_validate_t::root_last_hashed_index() const
{
    if (!root_vd_)
    {
        return {};
    }
    return root_versions().last_hashed_index;
}

const cpp2cpp_validate_t::aggregate_versions_t& cpp2cpp_validate_t::root_versions() const
{
    const auto found = aggregate_versions_.try_find_by<by_aggregate_vd>(*root_vd_);
    return found->get();
}

std::string cpp2cpp_validate_t::join_root_chunk_paths(const bool filename_only) const
{
    strings_t files;
    for (const auto& aggregate : aggregate_versions_)
    {
        if (!aggregate.is_root || !aggregate.chunk_path)
        {
            continue;
        }

        files.push_back(filename_only
            ? aggregate.chunk_path->filename().string()
            : report_path(*aggregate.chunk_path)
        );
    }
    return boost::join(files, ",");
}

std::string cpp2cpp_validate_t::join_declaration_statuses(const aggregate_versions_t& aggregate) const
{
    strings_t parts;
    for (const auto& declaration : aggregate.declarations)
    {
        if (declaration_status(aggregate, declaration.index) == version_status_et::DELETED)
        {
            parts.push_back(std::format("{} deleted", declaration.index));
        }
        else
        {
            parts.push_back(std::to_string(declaration.index));
        }
    }

    return boost::join(parts, ",");
}

indices_t cpp2cpp_validate_t::version_declaration_indices()
{
    return declaration_indices(root_versions());
}

indices_t cpp2cpp_validate_t::version_definition_indices()
{
    return root_versions().definition_indices;
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
