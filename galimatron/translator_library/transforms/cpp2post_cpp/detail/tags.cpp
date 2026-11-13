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
#include "galimatron/translator_library/galimatron_config_declaration.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;
using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_cmp);
}

vertex_descriptors_t cpp2post_cpp_t::compile_tags_in_version(const compile_tags_in_version_t& args)
{
    const auto version_info = exception::augment_version(args.version_index);
    const auto& graph = args.graph;
    const auto& merged = args.compiled_state.merged_of(args.vd_aggregate);
    const auto& versions_entities = merged.entities;
    const auto version_mode = version_mode_of(graph[merged.versions.at(args.version_index)]);
    auto& compiled_state = args.compiled_state;

    // tags and historical deletions up to and including this version, in declaration order
    struct statement_t
    {
        entity_et entity_type;
        vertex_descriptor_t vd_entity;
        index_t version_index;
        index_t index_in_versions;
    };

    std::vector<statement_t> statements;
    std::size_t tag_count = 0;
    for (const auto& record : versions_entities)
    {
        if (!statement_reaches_version(
            graph,
            args.vd_aggregate,
            version_mode,
            args.version_index,
            record.version_index,
            record.vd_entity
        ))
        {
            continue;
        }
        if (record.entity_type != entity_et::TAG_STATEMENT
            && record.entity_type != entity_et::OPERATION_STATEMENT)
        {
            continue;
        }
        if (record.entity_type == entity_et::TAG_STATEMENT)
        {
            // once and local are visible in the version where they are written
            const auto& tag = get_entity<tag_statement_t>(graph[record.vd_entity]);
            if (tag.mode != tag_mode_et::INHERIT
                && (record.from_anonymous || record.version_index != args.version_index))
            {
                continue;
            }
            ++tag_count;
        }
        statements.push_back({
            .entity_type = record.entity_type,
            .vd_entity = record.vd_entity,
            .version_index = record.version_index,
            .index_in_versions = record.index_in_versions
        });
    }

    statements |= ranges::actions::sort(ranges::less{}, &statement_t::index_in_versions);

    args.log_func(std::format("merged tags: {}", tag_count));

    // historical deletion: from this point the tag is absent from the compiled state.
    // physical deletion is different.
    // a label cannot be deleted. deleting an absent tag is a warning in the log.
    auto resolved_name = [&](const statement_t& statement) -> name_t
    {
        if (statement.entity_type == entity_et::OPERATION_STATEMENT)
        {
            return get_entity<operation_statement_t>(graph[statement.vd_entity]).name;
        }

        const auto& tag = get_entity<tag_statement_t>(graph[statement.vd_entity]);
        return compiled_state.get_name_for_tag({
            .vd_tag = statement.vd_entity,
            .name = tag.name
        });
    };

    auto tag_is_present_before = [&](const statement_t& operation) -> bool
    {
        const auto name = resolved_name(operation);
        if (operation.index_in_versions == 0)
        {
            return false;
        }

        const auto earlier = versions_entities.range_by_name_up_to_index_in_versions(
            name,
            operation.index_in_versions - 1
        );
        bool present = false;
        for (const auto& record : earlier)
        {
            if (record.entity_type == entity_et::OPERATION_STATEMENT)
            {
                present = false;
                continue;
            }
            if (record.entity_type != entity_et::TAG_STATEMENT)
            {
                continue;
            }
            if (!statement_reaches_version(
                graph,
                args.vd_aggregate,
                version_mode,
                args.version_index,
                record.version_index,
                record.vd_entity
            ))
            {
                continue;
            }

            const auto& tag = get_entity<tag_statement_t>(graph[record.vd_entity]);
            if (tag.mode != tag_mode_et::INHERIT
                && (record.from_anonymous || record.version_index != args.version_index))
            {
                continue;
            }
            present = true;
        }
        return present;
    };

    for (const auto& statement : statements)
    {
        if (statement.entity_type != entity_et::OPERATION_STATEMENT
            || statement.version_index != args.version_index
            || !entity_belongs_to_aggregate(graph, statement.vd_entity, args.vd_aggregate))
        {
            continue;
        }

        const auto& operation = get_entity<operation_statement_t>(graph[statement.vd_entity]);
        const auto file_path = file_path_of_tag(graph, statement.vd_entity);
        const auto path_info = exception::augment_where({
            .path = file_path
        });
        const auto path_text = file_path ? report_path(*file_path) : std::string{};

        const auto label_range = versions_entities.range_by_entity_type_and_name_up_to_index_in_versions(
            entity_et::LABEL_STATEMENT,
            operation.name,
            statement.index_in_versions
        );
        const auto label_hits = ranges::count_if(label_range, [&](const auto& record)
        {
            return statement_reaches_version(
                graph,
                args.vd_aggregate,
                version_mode,
                args.version_index,
                record.version_index,
                record.vd_entity
            );
        });
        if (label_hits != 0)
        {
            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::cpp2post_cpp{},
                exception::e_what_info{
                    std::format("cannot delete label '{}': {} version {}",
                        operation.name,
                        path_text,
                        statement.version_index
                    )
                }
            );
        }

        if (!tag_is_present_before(statement))
        {
            fLogmeW("delete tag '{}': tag is not present: {} version {}",
                operation.name,
                path_text,
                statement.version_index
            );
        }
    }

    auto tag_refs = vertex_descriptors_t{};

    // walk from the end: the last declaration of a name wins, a deletion removes the tag
    find_and_process_unique(
        ranges::views::reverse(statements),
        [&](const auto& statement)
        {
            if (statement.entity_type == entity_et::OPERATION_STATEMENT)
            {
                return;
            }

            const auto vd_tag = statement.vd_entity;
            const auto& tag = get_entity<tag_statement_t>(graph[vd_tag]);

            compiled_state.notify_labels_for_tag({
                .vd_tag = vd_tag,
                .get_vd_labels = [&]
                {
                    return resolve_tag_name_labels_to_vd_labels(
                    {
                        .versions_entities = versions_entities,
                        .graph = graph,
                        .vd_aggregate = args.vd_aggregate,
                        .version_mode = version_mode,
                        .tag_index_in_versions = statement.index_in_versions,
                        .tag_labels = tag.labels,
                        .version_index = statement.version_index,
                        .file_path = file_path_of_tag(graph, vd_tag),
                    });
                }
            });

            tag_refs.push_back(vd_tag);
        },
        [&](const auto& statement)
        {
            if (statement.entity_type == entity_et::OPERATION_STATEMENT)
            {
                const auto& operation = get_entity<operation_statement_t>(graph[statement.vd_entity]);
                return operation.name;
            }

            const auto vd_tag = statement.vd_entity;
            const auto& tag = get_entity<tag_statement_t>(graph[vd_tag]);
            return compiled_state.get_name_for_tag({
                .vd_tag = vd_tag,
                .name = tag.name
            });
        }
    );

    return tag_refs;
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
