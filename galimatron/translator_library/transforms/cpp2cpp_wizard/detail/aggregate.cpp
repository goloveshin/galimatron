// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// global
#include "global/common_library/utils.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

namespace
{
// the new aggregate file sits next to the command file, or under directory when that is set
//
state::vertex_descriptor_t new_aggregate_chunk(
    state::builder::builder_t& builder,
    const state::holder::language_part_t::graph_t& graph,
    const state::vertex_descriptor_t command_chunk,
    const std::string& name,
    const name_ot& directory
)
{
    const auto& current = graph[command_chunk];
    auto path = current.location->path->parent_path();
    if (directory)
    {
        path /= *directory;
    }
    path /= name + ".gal";
    return builder.create_chunk({
        .root_node = builder.root_node(),
        .location = location_t{
            .path = path
        }
    });
}

// the stored source text still shows the declaration before this deletion
//
void clear_deleted_declaration_source(
    state::holder::language_part_t::graph_t& graph,
    const state::vertex_descriptor_t vd
)
{
    auto& vertex = graph[vd];
    vertex.user_formatting.reset();

    if (auto* declaration = std::get_if<state::aggregate_version_declaration_t>(&vertex.entity))
    {
        declaration->hash.reset();
    }
}

// a dynamic inherit tag is carried into the next version by wizard create version
//
bool is_carried_dynamic_tag(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd
)
{
    const auto& vertex = state.graph()[vd];
    if (vertex.entity_type != entity_et::TAG_STATEMENT)
    {
        return false;
    }

    const auto& tag = std::get<tag_statement_t>(vertex.entity);
    if (tag.mode != tag_mode_et::INHERIT)
    {
        return false;
    }
    return tag_uses_dynamic_label(state, vd);
}

// statements directly under a version definition. empty when the definition is absent
//
std::vector<vertex_descriptor_t> definition_children(
    const holder::language_part_t& state,
    const vertex_descriptor_ot vd
)
{
    if (!vd)
    {
        return {};
    }

    auto statements = state.statements_of(*vd);
    return
        statements
        | ranges::views::transform([](const auto& statement)
        {
            return statement.vd;
        })
        | ranges::to_vector
        ;
}

// one version body of the source, with its statements, ready to replay onto each new aggregate
//
struct copied_body_t
{
    index_ot index;
    index_ot owner;
    tag_mode_et mode = tag_mode_et::INHERIT;
    std::vector<vertex_descriptor_t> children;
};

// version bodies of the source, in file order, including a parked version before N
//
std::vector<copied_body_t> bodies_of(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate
)
{
    auto bodies = state.versions().bodies_in_order(vd_aggregate);
    return
        bodies
        | ranges::views::transform([&](const auto& body)
        {
            auto statements = state.statements_of(body.vd);
            auto children =
                statements
                | ranges::views::transform([](const auto& statement)
                {
                    return statement.vd;
                })
                | ranges::to_vector
                ;
            auto owner = body.owner;
            if (body.index)
            {
                const auto& entity = state.graph()[body.vd].entity;
                if (const auto* definition = std::get_if<aggregate_version_definition_t>(&entity))
                {
                    owner = definition->owner;
                }
            }
            return copied_body_t{
                .index = body.index,
                .owner = owner,
                .mode = mode_of_definition(state, body.vd),
                .children = std::move(children)
            };
        })
        | ranges::to_vector
        ;
}

// every declaration of this aggregate is deleted. an aggregate with no declaration is not
//
bool every_version_deleted(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate
)
{
    const auto declarations = state.versions().declarations_in_order(vd_aggregate);
    if (declarations.empty())
    {
        return false;
    }

    for (const auto& declaration : declarations)
    {
        if (declaration.status != version_status_et::DELETED)
        {
            return false;
        }
    }
    return true;
}

// a chunk with no aggregate left is not a file
//
bool chunk_keeps_an_aggregate(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_chunk,
    const vertex_descriptor_t vd_aggregate
)
{
    return state.aggregates().keeps_other(vd_chunk, vd_aggregate);
}

// the aggregate leaves its file. an empty file leaves the root
//
void remove_aggregate_chunk(
    holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate
)
{
    auto& graph = state.graph();
    const auto vd_chunk = graph[vd_aggregate].vd_parent;
    if (vd_chunk != vertex_descriptor_not_present && graph[vd_chunk].entity_type == entity_et::CHUNK)
    {
        state.detach_vertex(vd_chunk, vd_aggregate);
        if (!chunk_keeps_an_aggregate(state, vd_chunk, vd_aggregate))
        {
            const auto vd_root = graph[vd_chunk].vd_parent;
            if (vd_root != vertex_descriptor_not_present)
            {
                state.detach_vertex(vd_root, vd_chunk);
            }
        }
    }

    const auto declarations = state.versions().declarations_in_order(vd_aggregate);
    state.versions().drop_definitions(vd_aggregate);
    for (const auto& declaration : declarations)
    {
        state.versions().forget(vd_aggregate, declaration.index);
    }
    state.aggregates().forget(vd_aggregate);
}
}// namespace

void cpp2cpp_wizard_t::apply_wizard_create_root_aggregate()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_ROOT_AGGREGATE)
    {
        return;
    }

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };

    const auto name = std::get<wizard::create_aggregate_data_t>(command.data).names.front();
    const auto chunk_node = new_aggregate_chunk(
        builder,
        language_state_->graph(),
        command.chunk_node,
        name,
        {}
    );
    const auto root_aggregate = builder.create_root_aggregate({
        .chunk_node = chunk_node,
        .name = name,
        .location = {},
        .created_by_wizard = true
    });

    builder.create_root_aggregate_version_declarations({
        .root_aggregate_node = root_aggregate
    });

    builder.create_root_aggregate_version_definitions({
        .root_aggregate_node = root_aggregate
    });
}

void cpp2cpp_wizard_t::apply_wizard_create_aggregate()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_AGGREGATE)
    {
        return;
    }

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };

    const auto& data = std::get<wizard::create_aggregate_data_t>(command.data);
    const auto& names = data.names;
    const auto command_chunk = command.chunk_node;

    for (const auto& name : names)
    {
        const auto& graph = language_state_->graph();
        const auto chunk_node = new_aggregate_chunk(
            builder,
            graph,
            command_chunk,
            name,
            data.directory
        );

        const auto aggregate = builder.create_aggregate({
            .chunk_node = chunk_node,
            .name = name,
            .index = language_state_->aggregates().next_index(),
            .location = {},
            .created_by_wizard = true
        });

        const auto versions_node = builder.create_aggregate_version_declarations({
            .aggregate_node = aggregate
        });

        const auto root = language_state_->aggregates().root();
        const auto declarations = language_state_->versions().declarations_in_order(root.vd);
        for (index_t i = 0; i < declarations.size(); ++i)
        {
            const bool is_last = i + 1 == declarations.size();
            builder.create_aggregate_version_declaration({
                .versions_node = versions_node,
                .index = declarations[i].index,
                .status = is_last ? version_status_oet{} : version_status_oet{ version_status_et::DELETED },
                .comment = {},
                .hash = {}
            });
        }

        const auto definitions_node = builder.create_aggregate_version_definitions({
            .aggregate_node = aggregate
        });

        if (!declarations.empty())
        {
            builder.create_aggregate_version_definition({
                .versions_node = definitions_node,
                .index = declarations.back().index,
                .mode = data.mode
            });
        }
    }
}

// a fully deleted aggregate loses its chunk.
// an aggregate that still has a live version gets delete; in the last version.
// no name removes every fully deleted aggregate
//
void cpp2cpp_wizard_t::apply_wizard_delete_aggregate()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::DELETE_AGGREGATE)
    {
        return;
    }

    const auto data = std::get<wizard::create_aggregate_data_t>(command.data);
    auto names = data.names;
    if (names.empty())
    {
        names =
            language_state_->aggregates()
            | ranges::views::filter([&](const auto& record)
            {
                return !record.is_root && every_version_deleted(*language_state_, record.vd);
            })
            | ranges::views::transform([](const auto& record)
            {
                return record.name;
            })
            | ranges::to<strings_t>()
            ;
    }

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };

    for (const auto& name : names)
    {
        const auto* record = language_state_->aggregates().try_by_name(name);
        if (!record || record->is_root)
        {
            continue;
        }

        const auto vd_aggregate = record->vd;
        if (data.physically || every_version_deleted(*language_state_, vd_aggregate))
        {
            remove_aggregate_chunk(*language_state_, vd_aggregate);
            continue;
        }

        const auto declarations = language_state_->versions().declarations_in_order(vd_aggregate);
        const auto index = declarations.back().index;
        const auto vd_definition = *language_state_->versions().try_definition(vd_aggregate, index);

        builder.create_operation_statement({
            .version_definition_node = vd_definition,
            .index = language_state_->next_version_statement_index(vd_aggregate, index),
            .name = {},
            .is_version = true,
            .location = {}
        });
        builder.apply_aggregate_version_delete(vd_definition);

        if (const auto vd_declaration = language_state_->versions().try_declaration(vd_aggregate, index))
        {
            clear_deleted_declaration_source(language_state_->graph(), *vd_declaration);
        }
    }
}

// the new aggregate receives the source declarations and bodies. the source stays
//
void cpp2cpp_wizard_t::apply_wizard_copy_aggregate()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::COPY_AGGREGATE)
    {
        return;
    }

    const auto data = std::get<wizard::copy_aggregate_data_t>(command.data);
    const auto* source = language_state_->aggregates().try_by_name(data.source);
    if (!source || source->is_root)
    {
        return;
    }

    const auto vd_source = source->vd;
    const auto& graph = language_state_->graph();
    const auto declarations = language_state_->versions().declarations_in_order(vd_source);
    const auto bodies = bodies_of(*language_state_, vd_source);

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };

    for (const auto& name : data.names)
    {
        const auto chunk_node = new_aggregate_chunk(
            builder,
            graph,
            command.chunk_node,
            name,
            data.directory
        );
        const auto aggregate = builder.create_aggregate({
            .chunk_node = chunk_node,
            .name = name,
            .index = language_state_->aggregates().next_index(),
            .location = {},
            .created_by_wizard = true
        });
        const auto versions_node = builder.create_aggregate_version_declarations({
            .aggregate_node = aggregate
        });

        for (const auto& declaration : declarations)
        {
            string_ot hash;
            string_ot comment;
            if (const auto vd = language_state_->versions().try_declaration(vd_source, declaration.index))
            {
                const auto& stored = std::get<aggregate_version_declaration_t>(graph[*vd].entity);
                hash = stored.hash;
                comment = stored.comment;
            }
            builder.create_aggregate_version_declaration({
                .versions_node = versions_node,
                .index = declaration.index,
                .status = declaration.status,
                .comment = comment,
                .hash = hash
            });
        }

        if (bodies.empty())
        {
            continue;
        }

        const auto definitions_node = builder.create_aggregate_version_definitions({
            .aggregate_node = aggregate
        });
        for (const auto& body : bodies)
        {
            const auto vd_definition = builder.create_aggregate_version_definition({
                .versions_node = definitions_node,
                .index = body.index,
                .owner = body.owner,
                .created_by_wizard = false,
                .mode = body.mode
            });
            for (const auto vd_child : body.children)
            {
                builder.clone_under({
                    .source = vd_child,
                    .parent = vd_definition,
                    .tag_index = {},
                    .created_by_wizard = false
                });
            }
        }
    }
}

// moves this statement to the end, replacing an earlier one of the same name
//
void place_named(
    std::vector<vertex_descriptor_t>& chosen,
    const name_t& name,
    const vertex_descriptor_t vd,
    const auto& name_of
)
{
    chosen |= ranges::actions::remove_if([&](const vertex_descriptor_t existing)
    {
        return name_of(existing) == name;
    });
    chosen.push_back(vd);
}

// labels and tags still in effect on the last version.
// a later statement of the same name replaces the earlier one. a delete removes that tag.
// once and local stay in the version where they are written.
// a local or once version does not keep earlier statements.
// a dynamic tag carried into the new version is left out here
//
std::vector<vertex_descriptor_t> statements_in_effect(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate,
    const index_t previous_index
)
{
    const auto previous_mode = mode_of_version(state, vd_aggregate, previous_index);
    const bool keeps_earlier = previous_mode == tag_mode_et::INHERIT
        || previous_mode == tag_mode_et::OWN;
    const auto& graph = state.graph();

    std::vector<vertex_descriptor_t> labels;
    std::vector<vertex_descriptor_t> tags;
    std::vector<vertex_descriptor_t> other;

    for (const auto& declaration : state.versions().declarations_in_order(vd_aggregate))
    {
        if (declaration.index > previous_index)
        {
            break;
        }
        if (!keeps_earlier && declaration.index != previous_index)
        {
            continue;
        }
        if (declaration.status == version_status_et::DELETED)
        {
            continue;
        }

        for (const auto& statement : state.statements_in_order(vd_aggregate, declaration.index))
        {
            if (statement.entity_type == entity_et::LABEL_STATEMENT)
            {
                const auto& label = std::get<label_statement_t>(graph[statement.vd].entity);
                place_named(
                    labels,
                    label.name,
                    statement.vd,
                    [&](const vertex_descriptor_t vd)
                    {
                        return std::get<label_statement_t>(graph[vd].entity).name;
                    }
                );
                continue;
            }

            if (statement.entity_type == entity_et::TAG_STATEMENT)
            {
                const auto& tag = std::get<tag_statement_t>(graph[statement.vd].entity);
                if (tag.mode != tag_mode_et::INHERIT && declaration.index != previous_index)
                {
                    continue;
                }
                if (!tag.name)
                {
                    tags.push_back(statement.vd);
                    continue;
                }
                place_named(
                    tags,
                    *tag.name,
                    statement.vd,
                    [&](const vertex_descriptor_t vd)
                    {
                        const auto& stored = std::get<tag_statement_t>(graph[vd].entity);
                        return stored.name ? *stored.name : name_t{};
                    }
                );
                continue;
            }

            if (statement.entity_type == entity_et::OPERATION_STATEMENT)
            {
                const auto& operation = std::get<operation_statement_t>(graph[statement.vd].entity);
                if (!operation.is_version)
                {
                    tags |= ranges::actions::remove_if([&](const vertex_descriptor_t vd)
                    {
                        const auto& tag = std::get<tag_statement_t>(graph[vd].entity);
                        return tag.name && *tag.name == operation.name;
                    });
                }
                continue;
            }

            if (declaration.index == previous_index)
            {
                other.push_back(statement.vd);
            }
        }
    }

    const auto kept_tags =
        tags
        | ranges::views::filter([&](const vertex_descriptor_t vd)
        {
            return !is_carried_dynamic_tag(state, vd);
        })
        | ranges::to_vector
        ;

    return ranges::views::concat(labels, kept_tags, other) | ranges::to_vector;
}

// the source ends in the new version. each new aggregate continues there, with the earlier versions deleted
//
void cpp2cpp_wizard_t::apply_wizard_continue_aggregate()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CONTINUE_AGGREGATE)
    {
        return;
    }

    const auto data = std::get<wizard::continue_aggregate_data_t>(command.data);
    const auto* source = language_state_->aggregates().try_by_name(data.source);
    if (!source || source->is_root)
    {
        return;
    }

    const auto vd_source = source->vd;
    const auto source_declarations = language_state_->versions().declarations_in_order(vd_source);
    if (source_declarations.empty())
    {
        return;
    }
    const auto previous_index = source_declarations.back().index;

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };
    const auto next_index = add_next_version(*language_state_, builder, tag_mode_et::INHERIT, false);
    const auto previous_mode = mode_of_version(*language_state_, vd_source, previous_index);
    const auto continued_mode = previous_mode == tag_mode_et::LOCAL
        ? tag_mode_et::LOCAL
        : previous_mode == tag_mode_et::OWN
            ? tag_mode_et::OWN
            : tag_mode_et::INHERIT;

    const auto& graph = language_state_->graph();
    const auto carried = definition_children(
        *language_state_,
        language_state_->versions().try_definition(vd_source, next_index)
    );
    const auto rest = statements_in_effect(*language_state_, vd_source, previous_index);

    const auto root_declarations = language_state_->versions().declarations_in_order(
        language_state_->aggregates().root().vd
    );

    for (const auto& name : data.names)
    {
        const auto chunk_node = new_aggregate_chunk(
            builder,
            graph,
            command.chunk_node,
            name,
            data.directory
        );
        const auto aggregate = builder.create_aggregate({
            .chunk_node = chunk_node,
            .name = name,
            .index = language_state_->aggregates().next_index(),
            .location = {},
            .created_by_wizard = true
        });
        const auto versions_node = builder.create_aggregate_version_declarations({
            .aggregate_node = aggregate
        });
        for (const auto& declaration : root_declarations)
        {
            const bool is_next = declaration.index == next_index;
            builder.create_aggregate_version_declaration({
                .versions_node = versions_node,
                .index = declaration.index,
                .status = is_next ? version_status_oet{} : version_status_oet{ version_status_et::DELETED },
                .comment = {},
                .hash = {}
            });
        }

        const auto definitions_node = builder.create_aggregate_version_definitions({
            .aggregate_node = aggregate
        });
        const auto vd_definition = builder.create_aggregate_version_definition({
            .versions_node = definitions_node,
            .index = next_index,
            .created_by_wizard = true,
            .mode = continued_mode
        });

        index_t statement_index = 0;
        for (const auto vd_child : carried)
        {
            builder.clone_under({
                .source = vd_child,
                .parent = vd_definition,
                .tag_index = statement_index,
                .created_by_wizard = true
            });
            ++statement_index;
        }
        for (const auto vd_child : rest)
        {
            builder.clone_under({
                .source = vd_child,
                .parent = vd_definition,
                .tag_index = statement_index,
                .created_by_wizard = true
            });
            ++statement_index;
        }
    }

    const auto vd_source_definition = language_state_->versions().try_definition(vd_source, next_index);
    if (!vd_source_definition)
    {
        return;
    }

    builder.create_operation_statement({
        .version_definition_node = *vd_source_definition,
        .index = language_state_->next_version_statement_index(vd_source, next_index),
        .name = {},
        .is_version = true,
        .location = {}
    });
    builder.apply_aggregate_version_delete(*vd_source_definition);

    if (const auto vd_declaration = language_state_->versions().try_declaration(vd_source, next_index))
    {
        clear_deleted_declaration_source(language_state_->graph(), *vd_declaration);
    }
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
