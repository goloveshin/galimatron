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
// drops a chosen tag when a later statement in this version deletes that name
//
void drop_tag_name(
    const holder::language_part_t& state,
    std::vector<vertex_descriptor_t>& chosen,
    const name_t& name
)
{
    chosen |= ranges::actions::remove_if([&](const vertex_descriptor_t vd)
    {
        const auto& tag = std::get<tag_statement_t>(state.graph()[vd].entity);
        return tag.name && *tag.name == name;
    });
}

// a local version does not inherit, so the new body is a copy of the previous body.
// once and local tags stay in the previous version
//
void copy_local_version_body(
    state::holder::language_part_t& state,
    state::builder::builder_t& builder,
    const state::vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    if (index == 0)
    {
        return;
    }

    const auto vd_new = state.versions().try_definition(vd_aggregate, index);
    if (!vd_new)
    {
        return;
    }

    index_ot previous_index;
    for (const auto& definition : state.versions().definitions_in_order(vd_aggregate))
    {
        if (definition.index < index)
        {
            previous_index = definition.index;
        }
    }
    if (!previous_index)
    {
        return;
    }

    const auto statements = state.statements_in_order(vd_aggregate, *previous_index);
    for (const auto& statement : statements)
    {
        const auto& entity = state.graph()[statement.vd].entity;
        if (statement.entity_type == state::entity_et::OPERATION_STATEMENT)
        {
            const auto& operation = std::get<state::operation_statement_t>(entity);
            if (operation.is_version)
            {
                continue;
            }
        }
        else if (statement.entity_type == state::entity_et::TAG_STATEMENT)
        {
            const auto& tag = std::get<state::tag_statement_t>(entity);
            if (tag.mode != state::tag_mode_et::INHERIT)
            {
                continue;
            }
        }
        else if (statement.entity_type != state::entity_et::LABEL_STATEMENT
            && statement.entity_type != state::entity_et::ORDERING_STATEMENT)
        {
            continue;
        }

        builder.clone_under({
            .source = statement.vd,
            .parent = *vd_new,
            .tag_index = {},
            .created_by_wizard = true
        });
    }
}

// tags that use a dynamic label are copied into the new version definition.
// a delete <name>; in the previous version keeps that tag out of this version and the ones after it
//
void copy_dynamic_tags(
    state::holder::language_part_t& state,
    state::builder::builder_t& builder,
    const state::vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    if (index == 0)
    {
        return;
    }

    const auto vd_new = state.versions().try_definition(vd_aggregate, index);
    if (!vd_new)
    {
        return;
    }

    index_ot previous_index;
    for (const auto& definition : state.versions().definitions_in_order(vd_aggregate))
    {
        if (definition.index < index)
        {
            previous_index = definition.index;
        }
    }
    if (!previous_index)
    {
        return;
    }

    const auto statements = state.statements_in_order(vd_aggregate, *previous_index);
    std::vector<vertex_descriptor_t> chosen;
    for (const auto& statement : statements)
    {
        const auto& entity = state.graph()[statement.vd].entity;
        if (statement.entity_type == state::entity_et::OPERATION_STATEMENT)
        {
            const auto& operation = std::get<state::operation_statement_t>(entity);
            // delete <name>; is the last word for that tag in this version
            if (!operation.is_version)
            {
                drop_tag_name(state, chosen, operation.name);
            }
            continue;
        }
        if (statement.entity_type != state::entity_et::TAG_STATEMENT)
        {
            continue;
        }
        if (!state::tag_uses_dynamic_label(state, statement.vd))
        {
            continue;
        }
        const auto& tag = std::get<state::tag_statement_t>(entity);
        // once and local exist in the version where they are written
        if (tag.mode != state::tag_mode_et::INHERIT)
        {
            continue;
        }
        // a later tag of the same name replaces the one a delete removed
        if (tag.name)
        {
            drop_tag_name(state, chosen, *tag.name);
        }
        chosen.push_back(statement.vd);
    }

    index_t next_index = 0;
    for (const auto vd_tag : chosen)
    {
        builder.clone_under({
            .source = vd_tag,
            .parent = *vd_new,
            .tag_index = next_index,
            .created_by_wizard = true
        });
        ++next_index;
    }
}
}// namespace

// the next version: in the root, a declaration and a definition,
// in the other aggregates a declaration like the previous one, a definition only if it is not deleted
//
index_t add_next_version(
    holder::language_part_t& state,
    state::builder::builder_t& builder,
    const tag_mode_et forced,
    const bool copy_local_body
)
{
    const auto root_record = state.aggregates().root();
    const auto root_node = root_record.vd;
    const auto root_declarations_node = *root_record.vd_declarations;

    const auto root_declarations = state.versions().declarations_in_order(root_node);
    const index_t index = root_declarations.empty() ? 0 : root_declarations.back().index + 1;
    const auto root_previous = root_declarations.empty()
        ? tag_mode_et::INHERIT
        : mode_of_version(state, root_node, root_declarations.back().index);
    const auto root_mode = mode_for_new_version(true, forced, root_previous);

    builder.create_root_aggregate_version_declaration({
        .versions_node = root_declarations_node,
        .index = index,
        .comment = {},
        .hash = {}
    });

    auto root_definitions = state.aggregates().root().vd_definitions;
    if (!root_definitions)
    {
        root_definitions = builder.create_root_aggregate_version_definitions({
            .root_aggregate_node = root_node
        });
    }
    builder.create_root_aggregate_version_definition({
        .versions_node = *root_definitions,
        .index = index,
        .created_by_wizard = true,
        .mode = root_mode
    });
    if (root_mode == tag_mode_et::LOCAL && copy_local_body)
    {
        copy_local_version_body(state, builder, root_node, index);
    }
    else
    {
        copy_dynamic_tags(state, builder, root_node, index);
    }

    for (const auto& record : state.aggregates())
    {
        if (record.is_root)
        {
            continue;
        }

        const auto vd_aggregate = record.vd;
        const auto versions_node = record.vd_declarations;
        const auto declarations = state.versions().declarations_in_order(vd_aggregate);
        const bool deleted = !declarations.empty() && declarations.back().status == version_status_et::DELETED;
        const auto previous = declarations.empty()
            ? tag_mode_et::INHERIT
            : mode_of_version(state, vd_aggregate, declarations.back().index);
        const auto mode = mode_for_new_version(false, forced, previous);

        builder.create_aggregate_version_declaration({
            .versions_node = *versions_node,
            .index = index,
            .status = deleted ? version_status_oet{ version_status_et::DELETED } : version_status_oet{},
            .comment = {},
            .hash = {}
        });

        if (deleted)
        {
            continue;
        }

        auto definitions = record.vd_definitions;
        if (!definitions)
        {
            definitions = builder.create_aggregate_version_definitions({
                .aggregate_node = vd_aggregate
            });
        }
        builder.create_aggregate_version_definition({
            .versions_node = *definitions,
            .index = index,
            .created_by_wizard = true,
            .mode = mode
        });
        if (mode == tag_mode_et::LOCAL && copy_local_body)
        {
            copy_local_version_body(state, builder, vd_aggregate, index);
        }
        else
        {
            copy_dynamic_tags(state, builder, vd_aggregate, index);
        }
    }

    return index;
}

// adds one version on the root and on every aggregate
//
void cpp2cpp_wizard_t::apply_wizard_create_version()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_VERSION)
    {
        return;
    }

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };
    const auto& data = std::get<wizard::create_version_data_t>(command.data);
    add_next_version(*language_state_, builder, data.mode);
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
