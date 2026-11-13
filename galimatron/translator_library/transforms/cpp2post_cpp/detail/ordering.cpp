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


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;

// a label statement is not named important, and its static values are unique
//
void cpp2post_cpp_t::check_labels_in_version(const check_labels_in_version_t& args)
{
    const auto& graph = args.graph;
    const auto& versions_entities = args.compiled_state.merged_of(args.vd_aggregate).entities;
    const auto labels = versions_entities.range_by_entity_type_and_version_index(
        entity_et::LABEL_STATEMENT,
        args.version_index
    );

    for (const auto& record : labels)
    {
        if (!entity_belongs_to_aggregate(graph, record.vd_entity, args.vd_aggregate))
        {
            continue;
        }

        const auto& label = get_entity<label_statement_t>(graph[record.vd_entity]);
        const auto where = exception::augment_where({
            .path = file_path_of_tag(graph, record.vd_entity)
        });

        if (label.name == name_t{ gal_and_xml_tags::label::important })
        {
            throw_compiler("label name 'important' is the built-in tag marker");
        }

        const auto static_range = args.graph_container.try_find_range_by_parent_and_entity_type(
            boost::make_tuple(record.vd_entity, entity_et::LABEL_STATIC)
        );
        for (const auto& stored : static_range)
        {
            const auto& static_part = get_entity<label_static_t>(stored);
            std::set<std::string> seen;
            for (const auto& value : static_part.values)
            {
                if (!seen.insert(value).second)
                {
                    throw_compiler(std::format("label '{}' has duplicate value '{}'", label.name, value));
                }
            }
            break;
        }
    }
}

// the parts are a strict weak order, and each name is a child aggregate of this version
// otherwise exception::domain::translator_library, exception::subdomain::translator_library::cpp2post_cpp
//
void cpp2post_cpp_t::check_ordering_names(const check_ordering_names_t& args)
{
    const auto where = exception::augment_where({
        .path = file_path_of_tag(args.graph, args.vd_ordering)
    });

    for (const auto& part : args.ordering.ordering_parts)
    {
        for (const auto& agg_name : part)
        {
            vertex_descriptor_ot child_vd;
            for (const auto& record : args.language.aggregates().try_find_range_by_name(agg_name))
            {
                if (!record.is_root)
                {
                    child_vd = record.vd;
                    break;
                }
            }

            if (!child_vd)
            {
                throw_compiler(std::format(
                    "ordering '{}' names '{}', which is not an aggregate",
                    args.ordering.name,
                    agg_name
                ));
            }

            const auto declared = args.language.versions().try_declaration(*child_vd, args.version_index);
            const auto status = args.language.versions().status(*child_vd, args.version_index);
            if (!declared || status == version_status_et::DELETED)
            {
                throw_compiler(std::format(
                    "ordering '{}' names '{}', which is absent from version {}",
                    args.ordering.name,
                    agg_name,
                    args.version_index
                ));
            }
        }
    }

    const auto ranks = ranks_of_ordering(args.ordering.ordering_parts);
    if (!ranks.problem)
    {
        return;
    }

    const auto& problem = *ranks.problem;
    if (problem.cycle && problem.first == problem.second)
    {
        throw_compiler(std::format(
            "ordering '{}' is not a strict weak order: '{}' precedes itself",
            args.ordering.name,
            problem.first
        ));
    }
    if (problem.cycle)
    {
        const auto left = problem.first < problem.second ? problem.first : problem.second;
        const auto right = problem.first < problem.second ? problem.second : problem.first;
        throw_compiler(std::format(
            "ordering '{}' is not a strict weak order: '{}' and '{}' precede each other",
            args.ordering.name,
            left,
            right
        ));
    }
    throw_compiler(std::format(
        "ordering '{}' is not a strict weak order: '{}' does not precede '{}'",
        args.ordering.name,
        problem.first,
        problem.second
    ));
}

void cpp2post_cpp_t::compile_orderings_in_version(const compile_orderings_in_version_t& args)
{
    const auto& graph = args.graph;
    const auto& versions_entities = args.compiled_state.merged_of(args.vd_aggregate).entities;
    auto& compiled_state = args.compiled_state;

    const auto version_mode = version_mode_of(graph[args.compiled_state.merged_of(args.vd_aggregate).versions.at(args.version_index)]);
    const auto ordering_range = versions_entities.range_by_entity_type_up_to_version_index(
        entity_et::ORDERING_STATEMENT,
        args.version_index
    );
    auto visible_orderings =
        ordering_range
        | ranges::views::filter([&](const auto& record)
        {
            return statement_reaches_version(
                graph,
                args.vd_aggregate,
                version_mode,
                args.version_index,
                record.version_index,
                record.vd_entity
            );
        })
        | ranges::to_vector
        ;

    // walk unique orderings from the end
    find_and_process_unique(
        ranges::views::reverse(visible_orderings),
        [&](const auto& record_ordering)
        {
            const auto vd_ordering = record_ordering.vd_entity;
            const auto& ordering = get_entity<ordering_statement_t>(graph[vd_ordering]);
            check_ordering_names({
                .graph = graph,
                .language = args.language,
                .vd_ordering = vd_ordering,
                .version_index = args.version_index,
                .ordering = ordering
            });
            compiled_state.notify_ordering_for_version({
                .version_index = args.version_index,
                .ordering = ordering
            });
        },
        [&](const auto& record_ordering)
        {
            return get_entity<ordering_statement_t>(graph[record_ordering.vd_entity]).name;
        }
    );
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
