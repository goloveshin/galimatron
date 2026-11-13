// galimatron
#include "galimatron/translator_library/translator/detail/xslt_seal_detail.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/common_library/exception_declaration.hpp"

// global
#include "global/common_library/utils.hpp"

// Boost.LEAF
#include <boost/leaf/exception.hpp>

namespace galimatron::translator::translator
{

using namespace galimatron::translator::state;

namespace
{

using graph_t = holder::language_part_t::graph_t;

struct declaration_fields_t
{
    string_ot hash;
    string_ot comment;
    version_status_oet status;
};

const holder::aggregate_container::detail::record_t* aggregate_record(
    holder::language_part_t& language,
    const vertex_descriptor_t vd_aggregate
)
{
    return language.aggregates().try_by_vd(vd_aggregate);
}

void write_declaration(graph_t& graph, const vertex_descriptor_t vd, const declaration_fields_t& fields)
{
    auto& vertex = graph[vd];
    if (auto* declaration = std::get_if<root_aggregate_version_declaration_t>(&vertex.entity))
    {
        declaration->hash = fields.hash;
        declaration->comment = fields.comment;
        return;
    }

    auto& declaration = std::get<aggregate_version_declaration_t>(vertex.entity);
    declaration.hash = fields.hash;
    declaration.comment = fields.comment;
    declaration.status = fields.status;
}

void copy_presentation(
    graph_t& edited,
    const graph_t& snapshot,
    const vertex_descriptor_t vd_edited,
    const vertex_descriptor_t vd_snapshot
)
{
    edited[vd_edited].user_formatting = snapshot[vd_snapshot].user_formatting;
    edited[vd_edited].user_comments = snapshot[vd_snapshot].user_comments;
}

void restore_one_version(
    builder::builder_t& builder,
    const holder::language_part_t& before,
    holder::language_part_t& after,
    const vertex_descriptor_t vd_before_aggregate,
    const vertex_descriptor_t vd_after_aggregate,
    const index_t version_index,
    const bool is_root
)
{
    const auto vd_before_decl = before.versions().try_declaration(vd_before_aggregate, version_index);
    if (vd_before_decl)
    {
        const auto stored = before.versions().try_stored(vd_before_aggregate, version_index);
        const auto fields = declaration_fields_t{
            .hash = stored->hash,
            .comment = stored->comment,
            .status = stored->status
        };
        auto vd_after_decl = after.versions().try_declaration(vd_after_aggregate, version_index);
        if (!vd_after_decl)
        {
            const auto versions_node = declarations_node(builder, after, vd_after_aggregate, is_root);
            if (is_root)
            {
                vd_after_decl = builder.create_root_aggregate_version_declaration({
                    .versions_node = versions_node,
                    .index = version_index,
                    .comment = fields.comment,
                    .hash = fields.hash
                });
            }
            else
            {
                vd_after_decl = builder.create_aggregate_version_declaration({
                    .versions_node = versions_node,
                    .index = version_index,
                    .status = fields.status,
                    .comment = fields.comment,
                    .hash = fields.hash
                });
            }
        }
        else
        {
            write_declaration(after.graph(), *vd_after_decl, fields);
            after.versions().set_declaration_fields(
                vd_after_aggregate,
                version_index,
                fields.hash,
                fields.comment,
                fields.status
            );
        }
        copy_presentation(after.graph(), before.graph(), *vd_after_decl, *vd_before_decl);
    }

    const auto vd_before_def = before.versions().try_definition(vd_before_aggregate, version_index);
    auto vd_after_def = after.versions().try_definition(vd_after_aggregate, version_index);
    if (!vd_before_def)
    {
        if (vd_after_def)
        {
            after.detach_children(*vd_after_def);
        }
        return;
    }

    if (!vd_after_def)
    {
        const auto versions_node = definitions_node(builder, after, vd_after_aggregate, is_root);
        if (is_root)
        {
            vd_after_def = builder.create_root_aggregate_version_definition({
                .versions_node = versions_node,
                .index = version_index
            });
        }
        else
        {
            vd_after_def = builder.create_aggregate_version_definition({
                .versions_node = versions_node,
                .index = version_index
            });
        }
    }
    else
    {
        after.detach_children(*vd_after_def);
    }

    std::vector<vertex_descriptor_t> children;
    for (const auto vd_child : subrange(boost::adjacent_vertices(*vd_before_def, before.graph())))
    {
        children.push_back(vd_child);
    }
    for (const auto vd_child : children)
    {
        builder.clone_from({
            .source_graph = before.graph(),
            .source = vd_child,
            .parent = *vd_after_def,
            .tag_index = {},
            .created_by_wizard = {},
            .clear_formatting = false
        });
    }
    copy_presentation(after.graph(), before.graph(), *vd_after_def, *vd_before_def);
}
}// namespace

const holder::aggregate_container::detail::record_t* find_aggregate(
    const holder::aggregate_container::container_t& aggregates,
    const name_t& name
)
{
    return aggregates.try_by_name(name);
}

index_ot last_sealed_index(const holder::language_part_t& before)
{
    return before.aggregates().root().last_hashed;
}

[[noreturn]] void throw_seal(const std::string& what)
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::translator{},
        exception::e_what_info{ what }
    );
}

void reject_sealed_shape(
    const holder::language_part_t& before,
    const holder::language_part_t& after,
    const index_t last_sealed
)
{
    for (const auto& record : before.aggregates())
    {
        if (!find_aggregate(after.aggregates(), record.name))
        {
            throw_seal(std::format("xslt removed aggregate '{}', which belongs to the sealed history", record.name));
        }
    }

    const auto before_root = before.aggregates().root().vd;
    const auto after_root = after.aggregates().root().vd;
    for (const auto& declaration : after.versions().declarations_in_order(after_root))
    {
        if (declaration.index > last_sealed)
        {
            continue;
        }
        if (!before.versions().try_declaration(before_root, declaration.index))
        {
            throw_seal(std::format("xslt inserted version {} into the sealed history", declaration.index));
        }
    }
}

bool sealed_history_differs(
    const holder::language_part_t& before,
    const holder::language_part_t& after,
    const index_t last_sealed
)
{
    const auto before_root = before.aggregates().root().vd;
    for (const auto& declaration : before.versions().declarations_in_order(before_root))
    {
        if (declaration.index > last_sealed)
        {
            continue;
        }

        for (const auto& record : before.aggregates())
        {
            const auto* edited = find_aggregate(after.aggregates(), record.name);
            const auto before_stored = before.versions().try_stored(record.vd, declaration.index);
            const auto after_stored = edited
                ? after.versions().try_stored(edited->vd, declaration.index)
                : std::optional<holder::version_container::container_t::stored_t>{};
            const bool before_declaration = before_stored && before_stored->has_declaration;
            const bool after_declaration = after_stored && after_stored->has_declaration;
            if (before_declaration != after_declaration)
            {
                return true;
            }
            if (before_declaration
                && (before_stored->hash != after_stored->hash
                    || before_stored->comment != after_stored->comment
                    || before_stored->status != after_stored->status))
            {
                return true;
            }

            const std::string before_text = before_stored ? before_stored->definition_payload : std::string{};
            const std::string after_text = after_stored ? after_stored->definition_payload : std::string{};
            if (before_text != after_text)
            {
                return true;
            }
        }
    }

    for (const auto& record : after.aggregates())
    {
        if (find_aggregate(before.aggregates(), record.name))
        {
            continue;
        }
        for (const auto& declaration : after.versions().declarations_in_order(record.vd))
        {
            if (declaration.index > last_sealed)
            {
                continue;
            }
            const auto stored = after.versions().try_stored(record.vd, declaration.index);
            if (stored && !stored->definition_payload.empty())
            {
                return true;
            }
        }
    }

    return false;
}

vertex_descriptor_t declarations_node(
    builder::builder_t& builder,
    holder::language_part_t& edited,
    const vertex_descriptor_t vd_aggregate,
    const bool is_root
)
{
    if (const auto* record = aggregate_record(edited, vd_aggregate))
    {
        if (record->vd_declarations)
        {
            return *record->vd_declarations;
        }
    }
    if (is_root)
    {
        return builder.create_root_aggregate_version_declarations({
            .root_aggregate_node = vd_aggregate
        });
    }
    return builder.create_aggregate_version_declarations({
        .aggregate_node = vd_aggregate
    });
}

vertex_descriptor_t definitions_node(
    builder::builder_t& builder,
    holder::language_part_t& edited,
    const vertex_descriptor_t vd_aggregate,
    const bool is_root
)
{
    if (const auto* record = aggregate_record(edited, vd_aggregate))
    {
        if (record->vd_definitions)
        {
            return *record->vd_definitions;
        }
    }
    if (is_root)
    {
        return builder.create_root_aggregate_version_definitions({
            .root_aggregate_node = vd_aggregate
        });
    }
    return builder.create_aggregate_version_definitions({
        .aggregate_node = vd_aggregate
    });
}

void restore_sealed_history(
    const holder::language_part_t& before,
    holder::language_part_t& after,
    const index_t last_sealed
)
{
    builder::builder_t builder{
        after,
        builder::builder_t::on_existing_state_t{}
    };

    const auto before_root = before.aggregates().root().vd;
    for (const auto& declaration : before.versions().declarations_in_order(before_root))
    {
        if (declaration.index > last_sealed)
        {
            continue;
        }
        for (const auto& record : before.aggregates())
        {
            const auto* edited = find_aggregate(after.aggregates(), record.name);
            restore_one_version(
                builder,
                before,
                after,
                record.vd,
                edited->vd,
                declaration.index,
                record.is_root
            );
        }
    }

    for (const auto& record : after.aggregates())
    {
        if (find_aggregate(before.aggregates(), record.name))
        {
            continue;
        }
        for (const auto& declaration : after.versions().declarations_in_order(record.vd))
        {
            if (declaration.index > last_sealed)
            {
                continue;
            }
            if (const auto vd = after.versions().try_definition(record.vd, declaration.index))
            {
                after.detach_children(*vd);
            }
        }
    }
}

}// namespace galimatron::translator::translator
