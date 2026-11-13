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

namespace
{
// writes the resolved name onto each unnamed tag so later lookups share one name index.
// root runs before other aggregates copy its entities
//
void remember_resolved_tag_names(
    const holder::language_part_t::graph_t& graph,
    holder::compiled_part_t& compiled,
    const vertex_descriptor_t vd_aggregate
)
{
    auto& entities = compiled.merged_of(vd_aggregate).entities;
    auto unnamed_range = entities.try_find_range_by_entity_type_and_name(
        boost::make_tuple(entity_et::TAG_STATEMENT, name_ot{})
    );
    auto unnamed =
        unnamed_range
        | ranges::views::transform([](const auto& record)
        {
            return record.vd_entity;
        })
        | ranges::to_vector
        ;

    for (const auto vd_tag : unnamed)
    {
        const auto name = compiled.get_name_for_tag({
            .vd_tag = vd_tag,
            .name = get_entity<tag_statement_t>(graph[vd_tag]).name
        });
        entities.note_name(vd_tag, name);
    }
}


// zero-level strings of a tag, in value order
//
strings_t strings_of_tag(
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptor_t vd_tag
)
{
    const auto& tag = get_entity<tag_statement_t>(graph[vd_tag]);
    auto pairs = ranges::subrange(tag.string_values_at_zero_level.begin(), tag.string_values_at_zero_level.end());
    return
        pairs
        | ranges::views::transform([&](const auto& pair)
        {
            return get_entity<tag_string_t>(graph[pair.second]).value;
        })
        | ranges::to<strings_t>()
        ;
}

// the tag value visible just before this statement. empty when the tag is absent
//
std::optional<strings_t> value_before_statement(
    const holder::language_part_t::graph_t& graph,
    const holder::version_entity_container::container_t& entities,
    const vertex_descriptor_t vd_aggregate,
    const tag_mode_et version_mode,
    const index_t version_index,
    const index_t index_in_versions,
    const name_t& name
)
{
    if (index_in_versions == 0)
    {
        return {};
    }

    const auto earlier = entities.range_by_name_up_to_index_in_versions(name, index_in_versions - 1);
    for (auto it = earlier.end(); it != earlier.begin();)
    {
        --it;
        const auto& record = *it;
        if (!statement_reaches_version(
            graph,
            vd_aggregate,
            version_mode,
            version_index,
            record.version_index,
            record.vd_entity
        ))
        {
            continue;
        }
        if (record.entity_type == entity_et::OPERATION_STATEMENT)
        {
            const auto& operation = get_entity<operation_statement_t>(graph[record.vd_entity]);
            if (!operation.is_version)
            {
                return {};
            }
            continue;
        }
        if (record.entity_type != entity_et::TAG_STATEMENT)
        {
            continue;
        }

        const auto& tag = get_entity<tag_statement_t>(graph[record.vd_entity]);
        const auto visible = tag.mode == tag_mode_et::INHERIT
            || record.version_index == version_index
            || record.version_index + 1 == version_index;
        if (visible)
        {
            return strings_of_tag(graph, record.vd_entity);
        }
    }
    return {};
}

// one pass records the value history later reads, so history does not replay tag visibility
//
void remember_values_before(
    holder::compiled_part_t& compiled,
    const holder::language_part_t::graph_t& graph,
    const vertex_descriptor_t vd_aggregate
)
{
    const auto& entities = compiled.merged_of(vd_aggregate).entities;
    for (const auto& record : entities)
    {
        if (!record.name)
        {
            continue;
        }
        if (record.entity_type == entity_et::OPERATION_STATEMENT)
        {
            const auto& operation = get_entity<operation_statement_t>(graph[record.vd_entity]);
            if (operation.is_version)
            {
                continue;
            }
        }
        else if (record.entity_type != entity_et::TAG_STATEMENT)
        {
            continue;
        }

        const auto version_vd = compiled.merged_of(vd_aggregate).versions.find(record.version_index);
        const auto version_mode = version_vd == compiled.merged_of(vd_aggregate).versions.end()
            ? tag_mode_et::INHERIT
            : version_mode_of(graph[version_vd->second]);
        compiled.remember_value_before(
            record.vd_entity,
            value_before_statement(
                graph,
                entities,
                vd_aggregate,
                version_mode,
                record.version_index,
                record.index_in_versions,
                *record.name
            )
        );
    }
}
}// namespace

void cpp2post_cpp_t::compile_merged_versions_entities(const compile_merged_versions_entities_t& args)
{
    const auto& graph = args.graph;
    const auto& merged = args.compiled_state.merged_of(args.vd_aggregate);
    const auto& version_index2vd_version = merged.versions;
    auto& compiled_state = args.compiled_state;
    auto& tag_container = compiled_state.tag_container();

    const auto tab_1 = log_tabulator_t();

    fLogmeD("begin compile merged versions entities");

    // a tag without a name takes it from a label's tag-name-from
    naming_unnamed_tags({
        .graph = graph,
        .compiled_state = compiled_state,
        .vd_aggregate = args.vd_aggregate,
    });
    remember_resolved_tag_names(graph, compiled_state, args.vd_aggregate);
    remember_values_before(compiled_state, graph, args.vd_aggregate);

    // walk the versions by number
    for (const auto& [version_index, vd_version] : version_index2vd_version)
    {
        const auto version_info = exception::augment_version(version_index);
        const auto tab_2 = log_tabulator_t();

        fLogmeD("begin version '{}'", version_index);

        check_labels_in_version({
            .graph = graph,
            .graph_container = args.graph_container,
            .compiled_state = compiled_state,
            .vd_aggregate = args.vd_aggregate,
            .version_index = version_index,
        });

        {
            const auto tab_3 = log_tabulator_t();

            // get the tags in reverse order
            const auto tag_refs = compile_tags_in_version({
                .graph = graph,
                .compiled_state = compiled_state,
                .vd_aggregate = args.vd_aggregate,
                .version_index = version_index,
                .log_func = [&](const auto& str)
                {
                    fLogmeD("{}", str);
                },
            });

            for (const auto& rendered : template_engine::render(graph, tag_refs))
            {
                compiled_state.remember_rendered_string(vd_version, rendered.vd_string, rendered.value);
            }

            // add all the tags in forward order
            auto reversed = tag_refs | ranges::views::reverse;
            auto records =
                reversed
                | ranges::views::transform([&](const auto& vd_tag)
                {
                    return holder::tag_container::container_t::record_t{
                        .vd_version = vd_version,
                        .vd_tag = vd_tag,
                        .name = compiled_state.get_name_for_tag({
                            .vd_tag = vd_tag,
                            .name = std::get<tag_statement_t>(graph[vd_tag].entity).name
                        }),
                        .body = holder::vertex_payload::vertex(graph, vd_tag)
                    };
                })
                | ranges::to_vector
                ;
            tag_container.insert(tag_container.end(), records.begin(), records.end());
        }

        if (args.is_root_aggregate)
        {
            compile_orderings_in_version({
                .graph = graph,
                .compiled_state = compiled_state,
                .vd_aggregate = args.vd_aggregate,
                .version_index = version_index,
                .language = args.language,
            });
        }

        fLogmeD("end version '{}'", version_index);
    }

    fLogmeD("end compile merged versions entities");
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
