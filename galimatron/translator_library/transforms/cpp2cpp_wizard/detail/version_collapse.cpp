// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/state/statement_facts.hpp"
#include "galimatron/translator_library/state/tag_name_part.hpp"

// global
#include "global/common_library/utils.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

namespace
{
bool body_is_only_version_delete(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd
)
{
    return state.is_only_version_delete(vd);
}

void reparent_statement(
    holder::language_part_t& state,
    const vertex_descriptor_t vd_child,
    const vertex_descriptor_t vd_parent,
    const index_t index_in_version
)
{
    auto& graph = state.graph();
    const auto vd_old = graph[vd_child].vd_parent;
    state.forget_version_child(vd_child);

    set_index_in_version(graph[vd_child].entity, index_in_version);

    boost::remove_edge(vd_old, vd_child, graph);
    auto& vertices = state.graph_container();
    vertices.modify_at(vd_child, [&](auto& stored)
    {
        stored.m_property.vd_parent = vd_parent;
    });
    boost::add_edge(vd_parent, vd_child, graph);
    state.note_version_child(vd_child);
}

// vd_child is already a child of parent. it is placed immediately before vd_before
//
void order_child_before(
    holder::language_part_t& state,
    const vertex_descriptor_t vd_parent,
    const vertex_descriptor_t vd_child,
    const vertex_descriptor_t vd_before
)
{
    auto& graph = state.graph();
    vertex_descriptors_vector_t children;
    for (const auto vd : subrange(boost::adjacent_vertices(vd_parent, graph)))
    {
        children.push_back(vd);
    }
    for (const auto vd : children)
    {
        boost::remove_edge(vd_parent, vd, graph);
    }
    for (const auto vd : children)
    {
        if (vd == vd_child)
        {
            continue;
        }
        if (vd == vd_before)
        {
            boost::add_edge(vd_parent, vd_child, graph);
        }
        boost::add_edge(vd_parent, vd, graph);
    }
}

void detach_definition(
    holder::language_part_t& state,
    const vertex_descriptor_t vd_definition
)
{
    const auto vd_parent = state.graph()[vd_definition].vd_parent;
    state.detach_children(vd_definition);
    if (vd_parent == vertex_descriptor_not_present)
    {
        return;
    }
    state.detach_vertex(vd_parent, vd_definition);
}

// the label bodies looked up so far, by label name.
// the graph stays as it is while one cache is used
//
using label_cache_t = std::map<name_t, const label_static_t*>;

// the static body of a label statement, when that body has tag-name-from
//
const label_static_t* naming_body_of(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_label
)
{
    const label_static_t* found = nullptr;
    for (const auto vd_part : subrange(boost::adjacent_vertices(vd_label, state.graph())))
    {
        const auto* body = std::get_if<label_static_t>(&state.graph()[vd_part].entity);
        if (body && body->tag_name_from)
        {
            found = body;
        }
    }
    return found;
}

// walks every version of every aggregate. the last body found is the latest
//
const label_static_t* walk_latest_label_static(
    const holder::language_part_t& state,
    const name_t& name
)
{
    const label_static_t* found = nullptr;
    auto consider = [&](const vertex_descriptor_t vd_body)
    {
        for (const auto vd_child : subrange(boost::adjacent_vertices(vd_body, state.graph())))
        {
            const auto* label = std::get_if<label_statement_t>(&state.graph()[vd_child].entity);
            if (!label || label->name != name)
            {
                continue;
            }
            if (const auto* body = naming_body_of(state, vd_child))
            {
                found = body;
            }
        }
    };

    for (const auto& record : state.aggregates())
    {
        for (const auto& declaration : state.versions().declarations_in_order(record.vd))
        {
            for (const auto& anonymous : state.versions().anonymous_owned_by(record.vd, declaration.index))
            {
                consider(anonymous.vd_definition);
            }
            if (const auto vd = state.versions().try_definition(record.vd, declaration.index))
            {
                consider(*vd);
            }
        }
    }
    return found;
}

// the static body of the latest label with this name.
// a later version hides an earlier label.
// a name that one label statement carries needs no walk: that label is the latest
//
const label_static_t* latest_label_static(
    const holder::language_part_t& state,
    const name_t& name,
    label_cache_t& cache
)
{
    if (const auto cached = cache.find(name); cached != cache.end())
    {
        return cached->second;
    }

    const auto labels = state.label_statements_named(name);
    const label_static_t* found = nullptr;
    if (labels.size() > 1)
    {
        found = walk_latest_label_static(state, name);
    }
    else if (labels.size() == 1)
    {
        found = naming_body_of(state, labels.front());
    }
    cache.emplace(name, found);
    return found;
}

// an unnamed tag takes the name a label's tag-name-from points at.
// absent when that label is missing, so the tag is not paired with another
//
name_ot name_from_label(
    const holder::language_part_t& state,
    const tag_statement_t& tag,
    label_cache_t& cache
)
{
    name_ot chosen;
    for (const auto& label_name : tag.labels)
    {
        if (label_name == gal_and_xml_tags::label::important)
        {
            continue;
        }
        const auto* body = latest_label_static(state, label_name, cache);
        if (!body)
        {
            continue;
        }
        const auto part = tag_name_part(state.graph(), *body, tag);
        if (part.status != tag_name_part_t::status_et::FOUND || part.value->empty())
        {
            return {};
        }
        if (chosen && *chosen != *part.value)
        {
            return {};
        }
        chosen = *part.value;
    }
    return chosen;
}

// tags and the delete of a tag share a name. a label and an ordering do not share that name
//
struct collapse_key_t
{
    entity_et kind = entity_et::TAG_STATEMENT;
    name_t name;

    bool operator==(const collapse_key_t&) const = default;
};

struct collapse_key_hash_t
{
    std::size_t operator()(const collapse_key_t& key) const
    {
        return std::hash<name_t>{}(key.name) ^ (static_cast<std::size_t>(key.kind) * 0x9e3779b97f4a7c15ULL);
    }
};

// absent when this statement does not replace another.
// a version delete is not parked
//
std::optional<collapse_key_t> collapse_key(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd,
    label_cache_t& cache
)
{
    const auto& vertex = state.graph()[vd];
    const auto facts = statement_facts(vertex.entity);
    if (!facts)
    {
        return {};
    }

    switch (vertex.entity_type)
    {
        case entity_et::TAG_STATEMENT:
            if (facts->name)
            {
                return collapse_key_t{ entity_et::TAG_STATEMENT, *facts->name };
            }
            if (const auto named = name_from_label(state, get_entity<tag_statement_t>(vertex), cache))
            {
                return collapse_key_t{ entity_et::TAG_STATEMENT, *named };
            }
            return {};
        case entity_et::LABEL_STATEMENT:
            return collapse_key_t{ entity_et::LABEL_STATEMENT, *facts->name };
        case entity_et::ORDERING_STATEMENT:
            return collapse_key_t{ entity_et::ORDERING_STATEMENT, *facts->name };
        case entity_et::OPERATION_STATEMENT:
            if (!facts->names_tag)
            {
                return {};
            }
            return collapse_key_t{ entity_et::TAG_STATEMENT, *facts->name };
        default:
            return {};
    }
}

struct kept_statement_t
{
    vertex_descriptor_t vd = vertex_descriptor_not_present;
    // a later statement of the same name took its place
    bool is_hidden = false;
};

// the last statement of each name, in the order that last statement was written.
// a later statement hides the earlier one. a statement with no name stays
//
vertex_descriptors_vector_t last_variants(
    const holder::language_part_t& state,
    const vertex_descriptors_vector_t& sources
)
{
    label_cache_t labels;
    std::vector<kept_statement_t> kept;
    std::unordered_map<collapse_key_t, std::size_t, collapse_key_hash_t> latest;
    for (const auto vd_source : sources)
    {
        if (body_is_only_version_delete(state, vd_source))
        {
            continue;
        }
        for (const auto vd_child : subrange(boost::adjacent_vertices(vd_source, state.graph())))
        {
            const auto& entity = state.graph()[vd_child].entity;
            if (const auto* operation = std::get_if<operation_statement_t>(&entity))
            {
                if (operation->is_version)
                {
                    continue;
                }
            }
            if (const auto key = collapse_key(state, vd_child, labels))
            {
                if (const auto earlier = latest.find(*key); earlier != latest.end())
                {
                    kept[earlier->second].is_hidden = true;
                }
                latest.insert_or_assign(*key, kept.size());
            }
            kept.push_back({
                .vd = vd_child
            });
        }
    }

    return
        kept
        | ranges::views::filter([](const kept_statement_t& statement){ return !statement.is_hidden; })
        | ranges::views::transform([](const kept_statement_t& statement){ return statement.vd; })
        | ranges::to_vector
        ;
}

void mark_declaration_deleted(
    holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    const auto vd = state.versions().try_declaration(vd_aggregate, index);
    if (!vd)
    {
        return;
    }

    auto& graph = state.graph();
    auto& vertex = graph[*vd];
    vertex.user_formatting.reset();

    string_ot comment;
    if (auto* declaration = std::get_if<root_aggregate_version_declaration_t>(&vertex.entity))
    {
        comment = declaration->comment;
        declaration->status = version_status_et::DELETED;
        declaration->hash.reset();
    }
    else if (auto* declaration = std::get_if<aggregate_version_declaration_t>(&vertex.entity))
    {
        comment = declaration->comment;
        declaration->status = version_status_et::DELETED;
        declaration->hash.reset();
    }

    state.versions().set_declaration_fields(
        vd_aggregate,
        index,
        {},
        comment,
        version_status_et::DELETED
    );
}

// definition links are rebuilt after the bodies move.
// a version that is left untouched returns before that
//
void collapse_one_aggregate(
    holder::language_part_t& state,
    state::builder::builder_t& builder,
    const vertex_descriptor_t vd_aggregate,
    const index_t first,
    const index_t last,
    const index_t owner,
    const bool opens_a_version
)
{
    vertex_descriptors_vector_t sources;
    for (index_t index = first; index <= last; ++index)
    {
        for (const auto& anonymous : state.versions().anonymous_owned_by(vd_aggregate, index))
        {
            sources.push_back(anonymous.vd_definition);
        }
        if (const auto vd = state.versions().try_definition(vd_aggregate, index))
        {
            sources.push_back(*vd);
        }
    }

    vertex_descriptor_ot vd_park;
    vertex_descriptor_ot vd_owner;
    if (opens_a_version)
    {
        vd_park = state.versions().try_definition(vd_aggregate, owner);
    }
    else if (state.versions().status(vd_aggregate, owner) == version_status_et::DELETED)
    {
        vd_park = {};
    }
    else if (const auto found = state.versions().try_definition(vd_aggregate, owner))
    {
        vd_owner = found;
        const auto vd_definitions = state.graph()[*found].vd_parent;
        const auto* record = state.aggregates().try_by_vd(vd_aggregate);
        vd_park = record && record->is_root
            ? builder.create_root_aggregate_version_definition({
                .versions_node = vd_definitions,
                .index = {},
                .owner = owner,
                .created_by_wizard = true
            })
            : builder.create_aggregate_version_definition({
                .versions_node = vd_definitions,
                .index = {},
                .owner = owner,
                .created_by_wizard = true
            });
    }

    if (!vd_park)
    {
        if (!opens_a_version && !vd_owner
            && state.versions().status(vd_aggregate, owner) != version_status_et::DELETED)
        {
            return;
        }

        for (const auto vd_source : sources)
        {
            detach_definition(state, vd_source);
        }
        for (index_t index = first; index <= last; ++index)
        {
            mark_declaration_deleted(state, vd_aggregate, index);
        }
    }
    else
    {
        index_t next_index = 0;
        for (const auto vd_child : last_variants(state, sources))
        {
            reparent_statement(state, vd_child, *vd_park, next_index);
            ++next_index;
        }

        for (const auto vd_source : sources)
        {
            detach_definition(state, vd_source);
        }

        if (!opens_a_version)
        {
            order_child_before(
                state,
                state.graph()[*vd_owner].vd_parent,
                *vd_park,
                *vd_owner
            );
        }

        for (index_t index = first; index <= last; ++index)
        {
            mark_declaration_deleted(state, vd_aggregate, index);
        }
    }

    state.rebind_version_definitions(vd_aggregate);
}
}// namespace

// a range with a version after it parks into version before that version.
// a range that ends on the last version parks into a new version.
// the parked body keeps the last statement of each name
//
void cpp2cpp_wizard_t::apply_wizard_collapse_version()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::COLLAPSE_VERSION)
    {
        return;
    }

    const auto range = std::get<wizard::collapse_version_data_t>(command.data);
    const auto root_record = language_state_->aggregates().root();
    const auto root_declarations = language_state_->versions().declarations_in_order(root_record.vd);
    const bool opens_a_version = !root_declarations.empty() && range.last == root_declarations.back().index;
    const auto owner = range.last + 1;

    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };

    if (opens_a_version)
    {
        const auto root_node = root_record.vd;
        builder.create_root_aggregate_version_declaration({
            .versions_node = *root_record.vd_declarations,
            .index = owner,
            .comment = {},
            .hash = {}
        });

        auto root_definitions = language_state_->aggregates().root().vd_definitions;
        if (!root_definitions)
        {
            root_definitions = builder.create_root_aggregate_version_definitions({
                .root_aggregate_node = root_node
            });
        }
        builder.create_root_aggregate_version_definition({
            .versions_node = *root_definitions,
            .index = owner,
            .created_by_wizard = true
        });

        for (const auto& record : language_state_->aggregates())
        {
            if (record.is_root)
            {
                continue;
            }

            const auto declarations = language_state_->versions().declarations_in_order(record.vd);
            const bool deleted = !declarations.empty() && declarations.back().status == version_status_et::DELETED;
            builder.create_aggregate_version_declaration({
                .versions_node = *record.vd_declarations,
                .index = owner,
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
                    .aggregate_node = record.vd
                });
            }
            const auto collapsed_previous = declarations.empty()
                ? tag_mode_et::INHERIT
                : mode_of_version(*language_state_, record.vd, declarations.back().index);
            builder.create_aggregate_version_definition({
                .versions_node = *definitions,
                .index = owner,
                .created_by_wizard = true,
                .mode = collapsed_previous == tag_mode_et::LOCAL
                    ? tag_mode_et::LOCAL
                    : collapsed_previous == tag_mode_et::OWN
                        ? tag_mode_et::OWN
                        : tag_mode_et::INHERIT
            });
        }
    }

    const auto aggregates =
        language_state_->aggregates()
        | ranges::views::transform([](const auto& record){ return record.vd; })
        | ranges::to_vector
        ;

    for (const auto vd_aggregate : aggregates)
    {
        collapse_one_aggregate(
            *language_state_,
            builder,
            vd_aggregate,
            range.first,
            range.last,
            owner,
            opens_a_version
        );
    }
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
