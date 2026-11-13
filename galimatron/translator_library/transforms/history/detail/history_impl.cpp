// galimatron
#include "galimatron/translator_library/transforms/history/detail/history_impl.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/xml_tags.hpp"
#include "galimatron/translator_library/state/holder/graph/container.hpp"


namespace galimatron::translator::transforms::history::detail
{

using namespace galimatron::translator::state;

namespace
{

struct aggregate_row_t
{
    name_t name;
    vertex_descriptor_t vd;
    bool is_root = false;
    index_t order = 0;
};

struct placed_row_t
{
    aggregate_row_t row;
    name_ot group;
    index_t band = 0;
    index_t group_index = 0;
};

// zero-level strings of a tag, in value order
//
strings_t tag_strings(
    const holder::language_part_t::graph_container_t& vertices,
    const vertex_descriptor_t vd_tag
)
{
    const auto& tag = get_entity<tag_statement_t>(vertices[vd_tag]);
    auto pairs = ranges::subrange(tag.string_values_at_zero_level.begin(), tag.string_values_at_zero_level.end());
    return
        pairs
        | ranges::views::transform([&](const auto& pair)
        {
            return get_entity<tag_string_t>(vertices[pair.second]).value;
        })
        | ranges::to<strings_t>()
        ;
}

// the tag's name. an unnamed tag takes the name the compiler stored
//
name_t resolved_tag_name(
    holder::compiled_part_t& compiled,
    const holder::language_part_t::graph_container_t& vertices,
    const vertex_descriptor_t vd_tag
)
{
    const auto& tag = get_entity<tag_statement_t>(vertices[vd_tag]);
    if (tag.name)
    {
        return *tag.name;
    }

    // a print before compilation has no stored name
    if (const auto stored = compiled.try_name_for_tag(vd_tag))
    {
        return *stored;
    }
    return {};
}

// the value compilation stored for this statement. empty when the tag was absent
//
std::optional<strings_t> value_before(
    holder::compiled_part_t& compiled,
    const vertex_descriptor_t vd_statement
)
{
    if (const auto* stored = compiled.value_before(vd_statement))
    {
        return *stored;
    }
    return {};
}

void append_strings(pugi::xml_node parent, const char* element, const strings_t& values)
{
    auto node = parent.append_child(element);
    for (const auto& value : values)
    {
        node.append_child("string").text().set(value.c_str());
    }
}

void append_change(
    pugi::xml_node version_node,
    const name_t& aggregate,
    const char* kind,
    const name_t* name,
    const index_ot index,
    const std::optional<strings_t>& before,
    const std::optional<strings_t>& after,
    const name_t* group,
    const bool stamp_root
)
{
    auto change = version_node.append_child("change");
    change.append_attribute("aggregate") = aggregate.c_str();
    change.append_attribute("kind") = kind;
    if (name)
    {
        change.append_attribute("name") = name->c_str();
    }
    if (index)
    {
        const auto text = std::to_string(*index);
        change.append_attribute("index") = text.c_str();
    }
    if (group)
    {
        change.append_attribute("group") = group->c_str();
    }
    if (stamp_root)
    {
        change.append_attribute("root") = tag_c::bool_true.data();
    }
    if (before)
    {
        append_strings(change, "before", *before);
    }
    if (after)
    {
        append_strings(change, "after", *after);
    }
}

bool is_live(
    const holder::language_part_t& language,
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
)
{
    const auto stored = language.versions().try_stored(vd_aggregate, version_index);
    return stored && stored->has_declaration && !stored->status;
}

bool is_deleted(
    const holder::language_part_t& language,
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
)
{
    const auto stored = language.versions().try_stored(vd_aggregate, version_index);
    return stored && stored->status && *stored->status == version_status_et::DELETED;
}

// deleted declarations that precede the aggregate's first live version
//
bool is_born_deleted(
    const holder::language_part_t& language,
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
)
{
    if (!is_deleted(language, vd_aggregate, version_index))
    {
        return false;
    }
    const auto* aggregate = language.aggregates().try_by_vd(vd_aggregate);
    return aggregate && (!aggregate->first_live || version_index < *aggregate->first_live);
}

bool is_first_live(
    const holder::language_part_t& language,
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
)
{
    if (!is_live(language, vd_aggregate, version_index))
    {
        return false;
    }
    const auto* aggregate = language.aggregates().try_by_vd(vd_aggregate);
    return aggregate && aggregate->first_live && *aggregate->first_live == version_index;
}

// the first listed marker tag present on this aggregate at the end of the version. empty for the root
//
name_ot group_of(
    holder::compiled_part_t& compiled,
    const aggregate_row_t& aggregate,
    const index_t version_index,
    const strings_t& groups
)
{
    if (aggregate.is_root)
    {
        return {};
    }

    const auto vd_version = compiled.try_merged_version(aggregate.vd, version_index);
    if (!vd_version)
    {
        return {};
    }

    for (const auto& name : groups)
    {
        if (compiled.tag_container().contains(*vd_version, name))
        {
            return name;
        }
    }
    return {};
}

void append_aggregate(
    pugi::xml_node version_node,
    const holder::language_part_t& language,
    holder::compiled_part_t& compiled,
    const aggregate_row_t& aggregate,
    const index_t version_index,
    const name_ot& group,
    const bool stamp_root
)
{
    const name_t* const group_name = group ? &*group : nullptr;
    const auto stored = language.versions().try_stored(aggregate.vd, version_index);
    if (!stored || !stored->has_declaration)
    {
        return;
    }
    if (is_born_deleted(language, aggregate.vd, version_index))
    {
        return;
    }

    if (!aggregate.is_root && is_first_live(language, aggregate.vd, version_index))
    {
        append_change(version_node, aggregate.name, "create-aggregate", nullptr, {}, {}, {}, group_name, stamp_root);
    }

    bool emitted_delete_version = false;
    const auto& vertices = language.graph_container();
    for (const auto& statement : language.statements_in_order(aggregate.vd, version_index))
    {
        const auto index = index_ot{ statement.index_in_version };

        if (statement.entity_type == entity_et::OPERATION_STATEMENT)
        {
            const auto& operation = get_entity<operation_statement_t>(vertices[statement.vd]);
            if (operation.is_version)
            {
                append_change(version_node, aggregate.name, "delete-version", nullptr, index, {}, {}, group_name, stamp_root);
                emitted_delete_version = true;
                continue;
            }
            const auto before = value_before(compiled, statement.vd);
            append_change(version_node, aggregate.name, "delete", &operation.name, index, before, {}, group_name, stamp_root);
            continue;
        }

        if (statement.entity_type == entity_et::LABEL_STATEMENT)
        {
            const auto& label = get_entity<label_statement_t>(vertices[statement.vd]);
            append_change(version_node, aggregate.name, "label", &label.name, index, {}, {}, group_name, stamp_root);
            continue;
        }

        if (statement.entity_type == entity_et::ORDERING_STATEMENT)
        {
            const auto& ordering = get_entity<ordering_statement_t>(vertices[statement.vd]);
            append_change(version_node, aggregate.name, "ordering", &ordering.name, index, {}, {}, group_name, stamp_root);
            continue;
        }

        if (statement.entity_type != entity_et::TAG_STATEMENT)
        {
            continue;
        }

        const auto name = resolved_tag_name(compiled, vertices, statement.vd);
        const auto after = tag_strings(vertices, statement.vd);
        const auto before = value_before(compiled, statement.vd);
        const char* kind = before ? "replace" : "add";
        const auto after_values = std::optional<strings_t>{ after };
        append_change(
            version_node,
            aggregate.name,
            kind,
            &name,
            index,
            before,
            after_values,
            group_name,
            stamp_root
        );
    }

    if (is_deleted(language, aggregate.vd, version_index) && !emitted_delete_version)
    {
        append_change(version_node, aggregate.name, "delete-version", nullptr, {}, {}, {}, group_name, stamp_root);
    }
}

}// namespace


void build(
    pugi::xml_node parent,
    const holder::language_part_t& language,
    holder::compiled_part_t& compiled
)
{
    // no marker tags: aggregate order, root first, no @group
    const strings_t groups;

    auto history = parent.append_child(gal_and_xml_tags::document::history);
    if (!language.aggregates().has_root())
    {
        return;
    }

    const auto& aggregates = language.aggregates();
    auto aggregate_records = ranges::subrange(aggregates.begin(), aggregates.end());
    auto rows =
        aggregate_records
        | ranges::views::transform([](const auto& record)
        {
            return aggregate_row_t{
                .name = record.name,
                .vd = record.vd,
                .is_root = record.is_root,
                .order = record.is_root
                    ? index_t{ 0 }
                    : (record.index ? *record.index + 1 : std::numeric_limits<index_t>::max())
            };
        })
        | ranges::to_vector
        | ranges::actions::sort(ranges::less{}, &aggregate_row_t::order)
        ;

    const auto root = language.aggregates().root().vd;
    for (index_t version_index = 0; ; ++version_index)
    {
        const auto stored = language.versions().try_stored(root, version_index);
        if (!stored || !stored->has_declaration)
        {
            break;
        }
        auto version_node = history.append_child("version");
        const auto text = std::to_string(version_index);
        version_node.append_attribute("index") = text.c_str();

        auto placed =
            rows
            | ranges::views::transform([&](const aggregate_row_t& row)
            {
                const auto group = group_of(compiled, row, version_index, groups);
                const auto found = ranges::find(groups, group ? *group : name_t{});
                // root, then each history group in listed order, then aggregates in no group
                return placed_row_t{
                    .row = row,
                    .group = group,
                    .band = row.is_root
                        ? index_t{ 0 }
                        : (group ? index_t{ 1 } : index_t{ 2 }),
                    .group_index = group
                        ? index_t(ranges::distance(groups.begin(), found))
                        : index_t{ 0 }
                };
            })
            | ranges::to_vector
            | ranges::actions::sort(ranges::less{}, [](const placed_row_t& item)
            {
                return std::make_tuple(item.band, item.group_index, item.row.order);
            })
            ;
        for (const auto& item : placed)
        {
            append_aggregate(
                version_node,
                language,
                compiled,
                item.row,
                version_index,
                item.group,
                item.row.is_root
            );
        }
    }
}

}// namespace galimatron::translator::transforms::history::detail
