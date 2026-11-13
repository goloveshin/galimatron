// galimatron
#include "galimatron/translator_library/translator/detail/xslt_seal_detail.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>

namespace galimatron::translator::translator
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

namespace
{

struct tag_picture_t
{
    name_t name;
    std::string body;
    vertex_descriptor_t vd_tag;
};

// the tag was copied into this version by the wizard and uses a dynamic label
//
bool is_wizard_dynamic_copy(
    const holder::language_part_t& language,
    const name_t& aggregate_name,
    const index_t version_index,
    const name_t& tag_name
)
{
    const auto* record = language.aggregates().try_by_name(aggregate_name);
    if (!record)
    {
        return false;
    }

    for (const auto& statement : language.statements_in_order(record->vd, version_index))
    {
        if (statement.entity_type != entity_et::TAG_STATEMENT)
        {
            continue;
        }
        const auto& tag = std::get<tag_statement_t>(language.graph()[statement.vd].entity);
        if (!tag.name || *tag.name != tag_name)
        {
            continue;
        }
        return tag.created_by_wizard && tag_uses_dynamic_label(language, statement.vd);
    }
    return false;
}

std::vector<tag_picture_t> effective_tags(
    const holder::compiled_part_t& compiled,
    const vertex_descriptor_ot vd_version
)
{
    std::vector<tag_picture_t> pictures;
    if (!vd_version)
    {
        return pictures;
    }

    for (const auto& record : compiled.tag_container().try_find_range_by_vd_version(*vd_version))
    {
        pictures.push_back({
            .name = record.name,
            .body = record.body,
            .vd_tag = record.vd_tag
        });
    }
    return pictures;
}

std::vector<change_t> diff_tags(
    const std::vector<tag_picture_t>& before,
    const std::vector<tag_picture_t>& after
)
{
    std::map<name_t, std::string> before_body;
    for (const auto& picture : before)
    {
        before_body.insert_or_assign(picture.name, picture.body);
    }

    std::map<name_t, bool> after_names;
    std::vector<change_t> changes;
    for (const auto& picture : after)
    {
        after_names.insert_or_assign(picture.name, true);
        const auto found = before_body.find(picture.name);
        if (found == before_body.end() || found->second != picture.body)
        {
            changes.push_back({
                .kind = change_kind_et::REASSIGN,
                .name = picture.name,
                .vd_tag = picture.vd_tag
            });
        }
    }
    for (const auto& picture : before)
    {
        if (after_names.find(picture.name) == after_names.end())
        {
            changes.push_back({
                .kind = change_kind_et::DELETE_TAG,
                .name = picture.name,
                .vd_tag = {}
            });
        }
    }
    return changes;
}

vertex_descriptor_ot compiled_version(
    const holder::compiled_part_t& compiled,
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
)
{
    return compiled.try_merged_version(vd_aggregate, version_index);
}
}// namespace

// every sealed edit is a wizard copy the xslt is allowed to finish
//
bool only_wizard_dynamic_copies(
    const holder::language_part_t& language,
    const std::vector<aggregate_edit_t>& edits,
    const index_t version_index
)
{
    if (edits.empty())
    {
        return false;
    }

    for (const auto& edit : edits)
    {
        for (const auto& change : edit.changes)
        {
            if (change.kind != change_kind_et::REASSIGN)
            {
                return false;
            }
            if (!is_wizard_dynamic_copy(language, edit.name, version_index, change.name))
            {
                return false;
            }
        }
    }
    return true;
}

std::vector<aggregate_edit_t> effective_edits(
    const holder::language_part_sptr_t& before,
    const holder::language_part_sptr_t& after,
    const index_t last_sealed
)
{
    const auto before_compiled = transforms::cpp2post_cpp::translate(before);
    const auto after_compiled = transforms::cpp2post_cpp::translate(after);

    std::vector<aggregate_edit_t> edits;
    for (const auto& record : after->aggregates())
    {
        const auto* snapshot = find_aggregate(before->aggregates(), record.name);
        const auto before_vd = snapshot
            ? compiled_version(*before_compiled, snapshot->vd, last_sealed)
            : vertex_descriptor_ot{};
        const auto after_vd = compiled_version(*after_compiled, record.vd, last_sealed);
        auto changes = diff_tags(
            effective_tags(*before_compiled, before_vd),
            effective_tags(*after_compiled, after_vd)
        );
        if (changes.empty())
        {
            continue;
        }
        edits.push_back({
            .name = record.name,
            .changes = std::move(changes)
        });
    }
    return edits;
}

index_t open_next_version(holder::language_part_t& edited)
{
    builder::builder_t builder{
        edited,
        builder::builder_t::on_existing_state_t{}
    };

    const auto root_node = edited.aggregates().root().vd;
    const auto root_declarations_node = edited.aggregates().root().vd_declarations;
    if (!root_declarations_node)
    {
        throw_seal(std::string{ "root aggregate has no version declarations" });
    }

    const auto root_declarations = edited.versions().declarations_in_order(root_node);
    const index_t index = root_declarations.empty() ? 0 : root_declarations.back().index + 1;

    builder.create_root_aggregate_version_declaration({
        .versions_node = *root_declarations_node,
        .index = index,
        .comment = {},
        .hash = {}
    });

    const auto root_definitions = definitions_node(builder, edited, root_node, true);
    builder.create_root_aggregate_version_definition({
        .versions_node = root_definitions,
        .index = index,
        .created_by_wizard = true
    });

    for (const auto& record : edited.aggregates())
    {
        if (record.is_root)
        {
            continue;
        }

        const auto declarations = edited.versions().declarations_in_order(record.vd);
        const bool deleted = !declarations.empty()
            && declarations.back().status == version_status_et::DELETED;
        const auto versions_node = declarations_node(builder, edited, record.vd, false);
        builder.create_aggregate_version_declaration({
            .versions_node = versions_node,
            .index = index,
            .status = deleted ? version_status_oet{ version_status_et::DELETED } : version_status_oet{},
            .comment = {},
            .hash = {}
        });
        if (deleted)
        {
            continue;
        }

        const auto definitions = definitions_node(builder, edited, record.vd, false);
        builder.create_aggregate_version_definition({
            .versions_node = definitions,
            .index = index,
            .created_by_wizard = true
        });
    }

    fLogmeI("xslt changed a sealed version; open version {}", index);
    return index;
}

index_t destination_version(holder::language_part_t& edited, const index_t last_sealed)
{
    const auto root = edited.aggregates().root().vd;
    index_ot found;
    for (const auto& declaration : edited.versions().declarations_in_order(root))
    {
        if (declaration.index <= last_sealed)
        {
            continue;
        }
        if (!found || declaration.index < *found)
        {
            found = declaration.index;
        }
    }
    if (found)
    {
        return *found;
    }
    return open_next_version(edited);
}

void write_edits(
    holder::language_part_t& edited,
    const index_t version_index,
    const std::vector<aggregate_edit_t>& edits
)
{
    builder::builder_t builder{
        edited,
        builder::builder_t::on_existing_state_t{}
    };

    for (const auto& edit : edits)
    {
        const auto* record = find_aggregate(edited.aggregates(), edit.name);
        if (!record)
        {
            continue;
        }
        if (edited.versions().status(record->vd, version_index) == version_status_et::DELETED)
        {
            continue;
        }

        auto vd_definition = edited.versions().try_definition(record->vd, version_index);
        if (!vd_definition)
        {
            const auto versions_node = definitions_node(builder, edited, record->vd, record->is_root);
            if (record->is_root)
            {
                vd_definition = builder.create_root_aggregate_version_definition({
                    .versions_node = versions_node,
                    .index = version_index,
                    .created_by_wizard = true
                });
            }
            else
            {
                vd_definition = builder.create_aggregate_version_definition({
                    .versions_node = versions_node,
                    .index = version_index,
                    .created_by_wizard = true
                });
            }
        }

        auto next_index = edited.next_version_statement_index(record->vd, version_index);
        for (const auto& change : edit.changes)
        {
            if (edited.version_names_tag(record->vd, version_index, change.name))
            {
                continue;
            }
            if (change.kind == change_kind_et::DELETE_TAG)
            {
                builder.create_operation_statement({
                    .version_definition_node = *vd_definition,
                    .index = next_index,
                    .name = change.name,
                    .is_version = false,
                    .location = {}
                });
                fLogmeI("delete tag '{}' in version {}", change.name, version_index);
            }
            else if (change.vd_tag)
            {
                builder.clone_from({
                    .source_graph = edited.graph(),
                    .source = *change.vd_tag,
                    .parent = *vd_definition,
                    .tag_index = next_index,
                    .created_by_wizard = true,
                    .clear_formatting = true
                });
                fLogmeI("reassign tag '{}' in version {}", change.name, version_index);
            }
            ++next_index;
        }
    }
}

}// namespace galimatron::translator::translator
