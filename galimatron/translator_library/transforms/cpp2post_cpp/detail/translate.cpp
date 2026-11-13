// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/cpp2post_cpp_impl.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
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


holder::compiled_part_sptr_t cpp2post_cpp_t::translate()
{
    auto compiled_state = std::make_shared<holder::compiled_part_t>();

    compiled_state->register_get_static_part_of_label_func([&](const vertex_descriptor_t vd_label) -> holder::aggregated_label_static_t
    {
        const auto& label = get_entity<label_statement_t>(graph_[vd_label]);

        // find the label's static part
        const auto label_static_range = graph_container_
            .try_find_range_by_parent_and_entity_type(boost::make_tuple(vd_label, entity_et::LABEL_STATIC));

        const label_static_t* static_part = nullptr;
        for (const auto& stored : label_static_range)
        {
            static_part = &get_entity<label_static_t>(stored);
            break;
        }

        // a dynamic label has no static values; an empty list stays in range for value indexing
        if (!static_part)
        {
            static const label_static_t no_static_values;
            return {
                .name = label.name,
                .label_static = no_static_values
            };
        }

        return {
            .name = label.name,
            .label_static = *static_part
        };
    });

    const auto& language = *config_.language_state;

    // master aggregate
    const auto vd_root = language.aggregates().root().vd;
    const auto& root_aggregate = get_entity<root_aggregate_t>(graph_[vd_root]);

    fLogmeD("begin compiling root aggregate '{}'", root_aggregate.name);

    auto& root_merged = create_merged_versions_entities({
        .language = language,
        .graph_container = graph_container_,
        .versions = language.versions(),
        .vd_aggregate = vd_root,
        .root_merged = nullptr,
        .compiled_state = *compiled_state
    });
    {
        const auto aggregate_info = exception::augment_aggregate(root_aggregate.name);
        compile_merged_versions_entities({
            .graph = graph_,
            .graph_container = graph_container_,
            .compiled_state = *compiled_state,
            .vd_aggregate = vd_root,
            .is_root_aggregate = true,
            .language = language
        });
    }

    fLogmeD("end compiling root aggregate '{}'", root_aggregate.name);

    auto default_ordering = holder::compiled_part_t::aggregate_orderings_t{};

    // aggregates
    for (const auto& record : language.aggregates())
    {
        if (record.is_root)
        {
            continue;
        }

        const auto& aggregate = get_entity<aggregate_t>(graph_[record.vd]);

        fLogmeD("begin compiling aggregate '{}'", aggregate.name);

        default_ordering.push_back({
            .aggregate_name = aggregate.name,
            .index = aggregate.index
        });

        const auto aggregate_info = exception::augment_aggregate(aggregate.name);

        create_merged_versions_entities({
            .language = language,
            .graph_container = graph_container_,
            .versions = language.versions(),
            .vd_aggregate = record.vd,
            .root_merged = &root_merged,
            .compiled_state = *compiled_state
        });
        compile_merged_versions_entities({
            .graph = graph_,
            .graph_container = graph_container_,
            .compiled_state = *compiled_state,
            .vd_aggregate = record.vd,
            .is_root_aggregate = false,
            .language = language
        });

        fLogmeD("end compiling aggregate '{}'", aggregate.name);
    }

    compiled_state->notify_default_aggregates_ordering({
        .default_ordering = default_ordering
    });

    compiled_state->force_resolve_labels();

    return compiled_state;
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
