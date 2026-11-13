// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/cpp2post_cpp_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/template_engine.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/holder/ordering_rank.hpp"
#include "galimatron/translator_library/state/holder/graph/vertex_payload.hpp"
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_cmp);
}

cpp2post_cpp_t::cpp2post_cpp_t(const config_t& config)
    : config_(config)
    , graph_(config_.language_state->graph())
    , graph_container_(config_.language_state->graph_container())
{
}

void cpp2post_cpp_t::append_own_version_entities(const append_own_version_entities_t& args)
{
    const auto tab_1 = log_tabulator_t();

    fLogmeD("begin own entities");

    {
    const auto tab_2 = log_tabulator_t();

        const auto statements = args.language.statements_of(args.vd_version);

        for (const auto& statement : statements)
        {
            const auto& record_entity = args.language.graph()[statement.vd];
            // a version delete; is not included in tag compilation
            if (record_entity.entity_type == entity_et::OPERATION_STATEMENT
                && std::get<operation_statement_t>(record_entity.entity).is_version)
            {
                continue;
            }

            const auto name = [&] -> name_ot
            {
                const auto& ent = record_entity;
                if (ent.entity_type == entity_et::TAG_STATEMENT)
                {
                    return std::get<tag_statement_t>(ent.entity).name;
                }
                else if (ent.entity_type == entity_et::LABEL_STATEMENT)
                {
                    return std::get<label_statement_t>(ent.entity).name;
                }
                else if (ent.entity_type == entity_et::ORDERING_STATEMENT)
                {
                    return std::get<ordering_statement_t>(ent.entity).name;
                }
                else if (ent.entity_type == entity_et::OPERATION_STATEMENT)
                {
                    return std::get<operation_statement_t>(ent.entity).name;
                }
                return {};
            }();

            args.versions_entities.push_back({
                .name = name,
                .entity_type = record_entity.entity_type,
                .vd_entity = record_entity.vd_self,
                .version_index = args.version_index,
                .index_in_versions = args.index_in_versions++,
                .from_anonymous = args.from_anonymous
            });

            fLogmeD("append entity '{}' of '{}'", name.value_or(""), to_string(record_entity.entity_type));
        }
    }

    fLogmeD("end own entities");
}

void cpp2post_cpp_t::append_other_version_entities(const append_other_version_entities_t& args)
{
    const auto tab_1 = log_tabulator_t();

    fLogmeD("begin root entities");

    {
    const auto tab_2 = log_tabulator_t();

    const auto other_range = args.versions_entities_other.try_find_range_by_version_index(args.version_index);
    // local stays on the aggregate where it is written. once is copied with this version
    auto copied = other_range | ranges::views::filter([&](const auto& ent)
    {
        if (ent.entity_type != entity_et::TAG_STATEMENT)
        {
            return true;
        }
        const auto& tag = get_entity<tag_statement_t>(args.graph_container[ent.vd_entity]);
        return tag.mode != tag_mode_et::LOCAL;
    });

    const auto count_before = args.versions_entities.size();
    ranges::copy(
        copied
        | ranges::views::transform([&](const auto& ent)
        {
            auto copy = ent;
            copy.index_in_versions = args.index_in_versions++;
            return copy;
        }),
        std::back_inserter(args.versions_entities)
    );
    const auto count_after = args.versions_entities.size();

    const auto added_count = count_after - count_before;

    fLogmeD("add entities: {}", added_count);
    }

    fLogmeD("end root entities");
}

cpp2post_cpp_t::merged_versions_t& cpp2post_cpp_t::create_merged_versions_entities(const create_merged_versions_entities_t& args)
{
    auto& merged = args.compiled_state.add_merged(args.vd_aggregate);
    auto& versions = merged.versions;
    auto& versions_entities = merged.entities;

    const auto tab_1 = log_tabulator_t();

    fLogmeD("begin create merged versions entities");

    const bool is_root_aggregate = args.root_merged == nullptr;

    // index across all versions
    index_t index_in_versions = 0;

    auto append_anonymous = [&](const index_t version_index)
    {
        for (const auto& anonymous : args.versions.anonymous_owned_by(args.vd_aggregate, version_index))
        {
            append_own_version_entities({
                .language = args.language,
                .vd_version = anonymous.vd_definition,
                .version_index = version_index,
                .index_in_versions = index_in_versions,
                .versions_entities = versions_entities,
                .from_anonymous = true
            });
        }
    };

    auto append = [&](const vertex_descriptor_ot& vd_version, const index_t version_index)
    {
        const auto tab_2 = log_tabulator_t();

        fLogmeD("begin version '{}'", version_index);

        if (vd_version)
        {
            versions.emplace(version_index, vd_version.value());
        }

        // local and own are the statements of this aggregate. the root version is not copied
        const auto version_mode = vd_version
            ? version_mode_of(args.language.graph()[*vd_version])
            : tag_mode_et::INHERIT;
        const bool skip_root = version_mode == tag_mode_et::LOCAL || version_mode == tag_mode_et::OWN;
        if (!is_root_aggregate && !skip_root)
        {
            append_other_version_entities({
                .graph_container = args.graph_container,
                .versions_entities_other = args.root_merged->entities,
                .version_index = version_index,
                .index_in_versions = index_in_versions,
                .versions_entities = versions_entities
            });
        }

        // parked statements follow the root labels of this version, so a collapsed tag still sees its label
        append_anonymous(version_index);

        // add from the current version, if it exists
        if (vd_version)
        {
            append_own_version_entities({
                .language = args.language,
                .vd_version = vd_version.value(),
                .version_index = version_index,
                .index_in_versions = index_in_versions,
                .versions_entities = versions_entities
            });
        }

        fLogmeD("end version '{}'", version_index);
    };

    if (is_root_aggregate)
    {
        for (const auto& version : args.versions.definitions_in_order(args.vd_aggregate))
        {
            append(version.vd, version.index);
        }
    }
    else
    {
        for (const auto& version : args.root_merged->versions)
        {
            const auto version_index = version.first;
            auto vd_version = args.versions.try_definition(args.vd_aggregate, version_index);
            // delete; marks the version deleted: its definition is not included in compilation
            if (vd_version && args.versions.status(args.vd_aggregate, version_index) == version_status_et::DELETED)
            {
                vd_version = {};
            }
            append(vd_version, version_index);
        }
    }

    fLogmeD("end create merged versions entities");

    return merged;
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
