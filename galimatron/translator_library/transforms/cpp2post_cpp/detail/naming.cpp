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

// generated lexer
#include "g_lexer.h"


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;
using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_cmp);
}

vertex_descriptors_ot cpp2post_cpp_t::resolve_tag_name_labels_to_vd_labels(const resolve_tag_name_labels_to_vd_labels_t& args)
{
    const auto where = exception::augment_where({
        .path = args.file_path,
        .version = args.version_index
    });

    vertex_descriptors_t vd_labels;

    for (const auto& label_name : args.tag_labels)
    {
        if (label_name == gal_and_xml_tags::label::important)
        {
            continue;
        }

        const auto label_range = args.versions_entities.range_by_entity_type_and_name_up_to_index_in_versions(
            entity_et::LABEL_STATEMENT,
            label_name,
            args.tag_index_in_versions
        );

        vertex_descriptor_ot chosen_label;
        for (const auto& record : label_range)
        {
            if (!statement_reaches_version(
                args.graph,
                args.vd_aggregate,
                args.version_mode,
                args.version_index,
                record.version_index,
                record.vd_entity
            ))
            {
                continue;
            }
            chosen_label = record.vd_entity;
        }
        if (!chosen_label)
        {
            const auto path_text = args.file_path ? report_path(*args.file_path) : std::string{};

            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::cpp2post_cpp{},
                exception::e_what_info{
                    std::format("no label '{}' for tag: {} version {}",
                        label_name,
                        path_text,
                        args.version_index
                    )
                }
            );
        }

        vd_labels.push_back(*chosen_label);
    }

    if (!vd_labels.empty())
    {
        return vd_labels;
    }
    return {};
}

void cpp2post_cpp_t::name_tag_from_label(const name_tag_from_label_t& args)
{
    static const std::regex identifier_pattern(galimatron::g_lexer::identifier_pattern);
    // at least one naming label, and every naming label picks the same identifier

    name_ot chosen;
    name_ot chosen_by;

    if (args.aggregated_labels_static)
    {
        for (const auto& aggregated_label_static : args.aggregated_labels_static.value())
        {
            const auto& aggregated_label = aggregated_label_static.get();
            const auto& label = aggregated_label.label_static;

            if (!label.tag_name_from)
            {
                continue;
            }

            // find the index among the values
            auto it = ranges::find(label.values, label.tag_name_from.value());
            if (it == ranges::end(label.values))
            {
                throw_compiler(std::format("label '{}' has tag-name-from but it is not among its values", aggregated_label.name));
            }

            const auto index = ranges::distance(ranges::begin(label.values), it);
            // get the tag's first-level values
            auto it2 = args.tag.string_values_at_zero_level.find(index);
            if (it2 == args.tag.string_values_at_zero_level.end())
            {
                throw_compiler(std::format("tag '{}' has no value with index {} for tag's name", args.vd_tag, index));
            }

            const auto& value = get_entity<tag_string_t>(args.graph[it2->second]).value;

            if (!std::regex_match(value, identifier_pattern))
            {
                throw_compiler(std::format(
                    "tag '{}' has value {} with index {} for tag's name but it is not a identifier",
                    args.vd_tag,
                    value,
                    index
                ));
            }

            // several naming labels must pick the same name
            if (chosen && *chosen != value)
            {
                throw_compiler(std::format(
                    "labels '{}' and '{}' name the tag differently: {} and {}",
                    *chosen_by,
                    aggregated_label.name,
                    *chosen,
                    value
                ));
            }

            chosen = value;
            chosen_by = aggregated_label.name;
        }
    }

    if (!chosen)
    {
        throw_compiler(std::format("tag '{}' has no name and has no label with naming", args.vd_tag));
    }

    args.notify_name_for_tag(args.vd_tag, *chosen);
}

void cpp2post_cpp_t::naming_unnamed_tags(const naming_unnamed_tags_t& args)
{
    const auto& graph = args.graph;
    const auto& versions_entities = args.compiled_state.merged_of(args.vd_aggregate).entities;
    auto& compiled_state = args.compiled_state;

    const auto tab_1 = log_tabulator_t();

    const auto tag_range = versions_entities.try_find_range_by_entity_type_and_name(
        boost::make_tuple(entity_et::TAG_STATEMENT, name_ot{})
    );
    for (const auto& record_tag : tag_range)
    {
        const auto vd_tag = record_tag.vd_entity;
        const auto where = exception::augment_where({
            .path = file_path_of_tag(graph, vd_tag),
            .version = record_tag.version_index
        });
        const auto& tag = get_entity<tag_statement_t>(graph[vd_tag]);

        const auto& aggregated_labels_static = compiled_state.notify_labels_for_tag({
            .vd_tag = vd_tag,
            .get_vd_labels = [&]
            {
                const auto version_vd = args.compiled_state.merged_of(args.vd_aggregate).versions.find(record_tag.version_index);
                const auto version_mode = version_vd == args.compiled_state.merged_of(args.vd_aggregate).versions.end()
                    ? tag_mode_et::INHERIT
                    : version_mode_of(graph[version_vd->second]);
                return resolve_tag_name_labels_to_vd_labels(
                {
                    .versions_entities = versions_entities,
                    .graph = graph,
                    .vd_aggregate = args.vd_aggregate,
                    .version_mode = version_mode,
                    .tag_index_in_versions = record_tag.index_in_versions,
                    .tag_labels = tag.labels,
                    .version_index = record_tag.version_index,
                    .file_path = file_path_of_tag(graph, vd_tag)
                });
            }
        });

        name_tag_from_label({
            .graph = graph,
            .aggregated_labels_static = aggregated_labels_static,
            .vd_tag = vd_tag,
            .tag = tag,
            .notify_name_for_tag = [&](const auto vd_tag, const auto& name){
                compiled_state.notify_name_for_tag({
                    .vd_tag = vd_tag,
                    .name = name
                });
                fLogmeD("empty tag name -> '{}'", name);
            }
        });
    }
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
