// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2xml
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2x);
}


vertex_descriptors_t cpp2xml_t::childs_of(const vertex_descriptor_t vd) const
{
    const auto& graph = language_state_->graph();
    const auto entity_type = graph[vd].entity_type;
    const bool is_version = entity_type == entity_et::ROOT_AGGREGATE_VERSION_DEFINITION
        || entity_type == entity_et::AGGREGATE_VERSION_DEFINITION;

    // a compiled version prints the tags stored for it. labels are not in that list
    if (compiled_state_ && is_version)
    {
        const auto tag_range = compiled_state_.value()->tag_container()
            .try_find_range_by_vd_version(vd);
        return
            tag_range
            | ranges::views::transform([](const auto& record)
            {
                return record.vd_tag;
            })
            | ranges::to<vertex_descriptors_t>()
            ;
    }

    // a language version prints its statement list, in index order
    if (is_version)
    {
        const auto statements = language_state_->statements_of(vd);
        return
            statements
            | ranges::views::transform([](const auto& statement)
            {
                return statement.vd;
            })
            | ranges::to<vertex_descriptors_t>()
            ;
    }

    const auto edges = subrange(boost::adjacent_vertices(vd, graph));
    return
        edges
        | ranges::views::filter([&](const vertex_descriptor_t vd_child)
        {
            const auto child_type = graph[vd_child].entity_type;
            if (child_type == entity_et::WIZARD_STATEMENT)
            {
                return false;
            }
            if (compiled_state_
                && (child_type == entity_et::LABEL_STATEMENT
                    || child_type == entity_et::LABEL_DYNAMIC
                    || child_type == entity_et::LABEL_STATIC))
            {
                return false;
            }
            return true;
        })
        | ranges::to<vertex_descriptors_t>()
        ;
}

pugi_document_with_meanful_node_t cpp2xml_t::translate()
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::cpp2xml{}
    );

    if (args_.layout_table)
    {
        args_.layout_table->clear();
    }

    const auto& graph = language_state_->graph();

    index_t index_of_child_of_version_ = 0;// index of the entity in the version

    const auto visitor = overloads_t
    {
        [&](const root_t& args) -> pugi::xml_node
        {
            return create_root(args);
        },
        [&](const chunk_t& args) -> pugi::xml_node
        {
            return create_chunk(args);
        },
        [&](const root_aggregate_t& args) -> pugi::xml_node
        {
            return create_root_aggregate(args);
        },
        [&](const root_aggregate_version_declarations_t& args) -> pugi::xml_node
        {
            return create_root_aggregate_version_declarations(args);
        },
        [&](const root_aggregate_version_declaration_t& args) -> pugi::xml_node
        {
            return create_root_aggregate_version_declaration(args);
        },
        [&](const root_aggregate_version_definitions_t& args) -> pugi::xml_node
        {
            return create_root_aggregate_version_definitions(args);
        },
        [&](const root_aggregate_version_definition_t& args) -> pugi::xml_node
        {
            return create_root_aggregate_version_definition(args);
        },
        [&](const aggregate_t& args) -> pugi::xml_node
        {
            return create_aggregate(args);
        },
        [&](const aggregate_version_declarations_t& args) -> pugi::xml_node
        {
            return create_aggregate_version_declarations(args);
        },
        [&](const aggregate_version_declaration_t& args) -> pugi::xml_node
        {
            return create_aggregate_version_declaration(args);
        },
        [&](const aggregate_version_definitions_t& args) -> pugi::xml_node
        {
            return create_aggregate_version_definitions(args);
        },
        [&](const aggregate_version_definition_t& args) -> pugi::xml_node
        {
            return create_aggregate_version_definition(args);
        },
        [&](const tag_statement_t& args) -> pugi::xml_node
        {
            auto copy_args = args;
            {
                copy_args.index_in_version = index_of_child_of_version_++;
            }
            return create_tag_statement(copy_args);
        },
        [&](const tag_values_t& args) -> pugi::xml_node
        {
            return create_tag_values(args);
        },
        [&](const tag_value_t& args) -> pugi::xml_node
        {
            return create_tag_value(args);
        },
        [&](const tag_record_t& args) -> pugi::xml_node
        {
            return create_tag_record(args);
        },
        [&](const tag_keyvalue_t& args) -> pugi::xml_node
        {
            return create_tag_keyvalue(args);
        },
        [&](const tag_array_t& args) -> pugi::xml_node
        {
            return create_tag_array(args);
        },
        [&](const tag_string_t& args) -> pugi::xml_node
        {
            return create_tag_string(args);
        },
        [&](const label_statement_t& args) -> pugi::xml_node
        {
            auto copy_args = args;
            {
                copy_args.index_in_version = index_of_child_of_version_++;
            }
            return create_label_statement(copy_args);
        },
        [&](const label_dynamic_t& args) -> pugi::xml_node
        {
            return create_label_dynamic(args);
        },
        [&](const label_static_t& args) -> pugi::xml_node
        {
            return create_label_static(args);
        },
        [&](const ordering_statement_t& args) -> pugi::xml_node
        {
            auto copy_args = args;
            {
                copy_args.index_in_version = index_of_child_of_version_++;
            }
            return create_ordering_statement(copy_args);
        },
        [&](const operation_statement_t& args) -> pugi::xml_node
        {
            auto copy_args = args;
            {
                copy_args.index_in_version = index_of_child_of_version_++;
            }
            return create_operation_statement(copy_args);
        },
        [&](const wizard_statement_t&) -> pugi::xml_node
        {
            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::cpp2xml{},
                exception::e_what_info{ "wizard statement is not an xml node" }
            );
        }
    };

    auto create_node_by_vertex = [&](const vertex_t& vertex) -> pugi::xml_node
    {
        vertex_ = std::cref(vertex);

        auto node = std::visit(visitor, vertex.entity);

        if (vertex.entity_type == entity_et::CHUNK)
        {
            append_location({
                .parent_node = node,
                .location = vertex.location,
                .split_path_by_parts = args_.split_path_by_parts
            });
        }

        return node;
    };

    auto walk_childs = [&](const vertex_descriptor_t vd) -> void
    {
        auto walk_childs_impl = [&](auto& walk_childs_ref_for_recurse, const vertex_descriptor_t vd) -> void
        {
            const auto& vertex = graph[vd];

            const bool is_version = vertex.entity_type == entity_et::ROOT_AGGREGATE_VERSION_DEFINITION
                || vertex.entity_type == entity_et::AGGREGATE_VERSION_DEFINITION;
            if (is_version)
            {
                index_of_child_of_version_ = 0;
            }
            if (compiled_state_ && is_version)
            {
                vd_compiled_version_ = vd;
            }

            const auto node = create_node_by_vertex(vertex);

            push_parent(node);
            {
                for (const auto vd_child : childs_of(vd))
                {
                    walk_childs_ref_for_recurse(walk_childs_ref_for_recurse, vd_child);
                }
            }
            pop_parent();
            remember_layout(vertex, node);
            if (compiled_state_ && is_version)
            {
                vd_compiled_version_.reset();
            }
        };
        return walk_childs_impl(walk_childs_impl, vd);
    };

    walk_childs(language_state_->root_node());
    write_string_delimiters();

    return {
        .document = doc_,
        .meanful_node = mode_node_
    };
}

}// namespace galimatron::translator::transforms::cpp2xml
