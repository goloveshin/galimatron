// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/holder/ordering_rank.hpp"
#include "galimatron/translator_library/state/holder/containers/tag_container.hpp"
#include "galimatron/translator_library/state/holder/graph/vertex_payload.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"

// global
#include "global/common_library/utils.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state::holder
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_hld);
}


using namespace ranges;

language_part_t::language_part_t()
    : graph_container_(graph_.m_vertices)
{
    versions_.on_declaration_changed([this](const vertex_descriptor_t vd_aggregate)
    {
        refresh_live_and_hashed(vd_aggregate);
    });
    string_delimiters_.push_back({
        .raw = false,
        .quote = '"',
        .dashes = {}
    });
    fLogmeD("language state created");
}

// returns the index of this delimiter, appending it when it is new
//
index_t language_part_t::intern_string_delimiter(const string_delimiter_t& delimiter)
{
    const auto found = std::ranges::find(string_delimiters_, delimiter);
    if (found != string_delimiters_.end())
    {
        return static_cast<index_t>(std::distance(string_delimiters_.begin(), found));
    }
    string_delimiters_.push_back(delimiter);
    return string_delimiters_.size() - 1;
}

// the document's delimiter list. an empty list keeps the plain double quote
//
void language_part_t::replace_string_delimiters(std::vector<string_delimiter_t> delimiters)
{
    if (delimiters.empty())
    {
        string_delimiters_.clear();
        string_delimiters_.push_back({
            .raw = false,
            .quote = '"',
            .dashes = {}
        });
        return;
    }
    string_delimiters_ = std::move(delimiters);
}

language_part_t::~language_part_t()
{
    fLogmeD("language state destroyed");
}

namespace
{

void set_definition_owner(
    language_part_t::graph_t& graph,
    version_container::container_t& versions,
    const vertex_descriptor_t vd_definition,
    const index_t owner
)
{
    auto& entity = graph[vd_definition].entity;
    if (auto* definition = std::get_if<root_aggregate_version_definition_t>(&entity))
    {
        definition->owner = owner;
    }
    else if (auto* definition = std::get_if<aggregate_version_definition_t>(&entity))
    {
        definition->owner = owner;
    }
    versions.set_anonymous_owner(vd_definition, owner);
}

index_ot numbered_index_of(const entity_t& entity)
{
    if (const auto* definition = std::get_if<root_aggregate_version_definition_t>(&entity))
    {
        return definition->index;
    }
    if (const auto* definition = std::get_if<aggregate_version_definition_t>(&entity))
    {
        return definition->index;
    }
    return {};
}

index_ot written_owner_of(const entity_t& entity)
{
    if (const auto* definition = std::get_if<root_aggregate_version_definition_t>(&entity))
    {
        return definition->owner;
    }
    if (const auto* definition = std::get_if<aggregate_version_definition_t>(&entity))
    {
        return definition->owner;
    }
    return {};
}

bool is_version_definition(const entity_et type)
{
    return type == entity_et::ROOT_AGGREGATE_VERSION_DEFINITION
        || type == entity_et::AGGREGATE_VERSION_DEFINITION;
}

}// namespace


void language_part_t::refresh_version_records()
{
    version_statements_.clear();
    ordering_links_.clear();

    for (const auto& aggregate : aggregates_)
    {
        for (const auto& definition : versions_.definitions_in_order(aggregate.vd))
        {
            // anonymous bodies sit before their owner. their text is part of that sealed version
            std::string payload;
            for (const auto& anonymous : versions_.anonymous_owned_by(aggregate.vd, definition.index))
            {
                payload += vertex_payload::children(graph_, anonymous.vd_definition);
            }
            payload += vertex_payload::children(graph_, definition.vd);
            versions_.set_definition_payload(
                aggregate.vd,
                definition.index,
                payload
            );
            for (const auto vd_child : ::subrange(boost::adjacent_vertices(definition.vd, graph_)))
            {
                note_version_child(vd_child);
            }
        }
        for (const auto& anonymous : versions_.anonymous_in_order(aggregate.vd))
        {
            for (const auto vd_child : ::subrange(boost::adjacent_vertices(anonymous.vd_definition, graph_)))
            {
                note_version_child(vd_child);
            }
        }
    }
}

void language_part_t::bind_anonymous_owners(const vertex_descriptor_t vd_definitions)
{
    std::vector<vertex_descriptor_t> pending;
    for (const auto vd_child : ::subrange(boost::adjacent_vertices(vd_definitions, graph_)))
    {
        if (!is_version_definition(graph_[vd_child].entity_type))
        {
            continue;
        }
        if (const auto index = numbered_index_of(graph_[vd_child].entity))
        {
            for (const auto vd_anonymous : pending)
            {
                if (written_owner_of(graph_[vd_anonymous].entity))
                {
                    continue;
                }
                set_definition_owner(graph_, versions_, vd_anonymous, *index);
            }
            pending.clear();
            continue;
        }
        pending.push_back(vd_child);
    }
}

void language_part_t::rebind_version_definitions(const vertex_descriptor_t vd_aggregate)
{
    const auto* record = aggregates_.try_by_vd(vd_aggregate);
    if (!record || !record->vd_definitions)
    {
        return;
    }

    const auto vd_definitions = *record->vd_definitions;
    versions_.drop_definitions(vd_aggregate);

    std::vector<vertex_descriptor_t> pending;
    for (const auto vd_child : ::subrange(boost::adjacent_vertices(vd_definitions, graph_)))
    {
        if (!is_version_definition(graph_[vd_child].entity_type))
        {
            continue;
        }
        if (const auto index = numbered_index_of(graph_[vd_child].entity))
        {
            versions_.add_definition(vd_aggregate, *index, vd_child);
            for (const auto vd_anonymous : pending)
            {
                set_definition_owner(graph_, versions_, vd_anonymous, *index);
            }
            pending.clear();
            continue;
        }
        versions_.add_anonymous(vd_aggregate, vd_child);
        pending.push_back(vd_child);
    }
}

void language_part_t::note_version_child(const vertex_descriptor_t vd_child)
{
    const auto& child = graph_[vd_child];
    const auto vd_definition = child.vd_parent;
    if (vd_definition == vertex_descriptor_not_present)
    {
        return;
    }

    const auto& definition = graph_[vd_definition];
    const auto is_definition = definition.entity_type == entity_et::ROOT_AGGREGATE_VERSION_DEFINITION
        || definition.entity_type == entity_et::AGGREGATE_VERSION_DEFINITION;
    if (!is_definition)
    {
        return;
    }

    const auto version_index = numbered_index_of(definition.entity);
    const auto vd_definitions = definition.vd_parent;
    const auto vd_aggregate = graph_[vd_definitions].vd_parent;

    name_ot name;
    index_t index_in_version = 0;
    bool names_tag = false;
    const ordering_statement_t* ordering = nullptr;
    if (const auto* tag = std::get_if<tag_statement_t>(&child.entity))
    {
        name = tag->name;
        index_in_version = tag->index_in_version;
        names_tag = tag->name.has_value();
    }
    else if (const auto* label = std::get_if<label_statement_t>(&child.entity))
    {
        name = label->name;
        index_in_version = label->index_in_version;
    }
    else if (const auto* found = std::get_if<ordering_statement_t>(&child.entity))
    {
        ordering = found;
        name = found->name;
        index_in_version = found->index_in_version;
    }
    else if (const auto* operation = std::get_if<operation_statement_t>(&child.entity))
    {
        name = operation->name;
        index_in_version = operation->index_in_version;
        names_tag = !operation->is_version;
    }
    else
    {
        return;
    }

    const auto inserted = version_statements_.add({
        .vd_aggregate = vd_aggregate,
        .version_index = version_index,
        .vd_definition = vd_definition,
        .vd_entity = vd_child,
        .name = name,
        .index_in_version = index_in_version,
        .names_tag = names_tag,
        .entity_type = child.entity_type
    });
    if (!inserted || !ordering)
    {
        return;
    }
    if (!aggregates_.has_root() || aggregates_.root().vd != vd_aggregate)
    {
        return;
    }
    ordering_links_.add(vd_child, version_index, ordering->name, ordering->ordering_parts);
}

void language_part_t::forget_version_child(const vertex_descriptor_t vd_child)
{
    version_statements_.forget(vd_child);
    ordering_links_.forget(vd_child);
}

void language_part_t::detach_vertex(
    const vertex_descriptor_t vd_parent,
    const vertex_descriptor_t vd_child
)
{
    boost::remove_edge(vd_parent, vd_child, graph_);
    graph_container_.modify_at(vd_child, [&](auto& stored)
    {
        stored.m_property.vd_parent = vertex_descriptor_not_present;
    });
    forget_version_child(vd_child);
}

void language_part_t::detach_children(const vertex_descriptor_t vd_parent)
{
    const auto children = ::subrange(boost::adjacent_vertices(vd_parent, graph_)) | ranges::to_vector;
    for (const auto vd_child : children)
    {
        detach_vertex(vd_parent, vd_child);
    }
}

namespace drop_detached
{

void hold_kept_entities(
    const language_part_t& state,
    const std::vector<char>& kept,
    std::vector<vertex_descriptor_t>& held
)
{
    const auto count = kept.size();
    for (vertex_descriptor_t vd = 0; vd < count; ++vd)
    {
        if (!kept[vd])
        {
            continue;
        }
        const auto& entity = state.graph()[vd].entity;
        if (const auto* tag = std::get_if<tag_statement_t>(&entity))
        {
            for (const auto& value : tag->string_values_at_zero_level)
            {
                hold_vertex(held, value.second);
            }
        }
        if (const auto* wizard = std::get_if<wizard_statement_t>(&entity))
        {
            hold_vertex(held, wizard->chunk_node);
        }
    }
}

std::vector<char> vertices_to_keep(
    const language_part_t& state,
    const version_statement_container::container_t& statements,
    const ordering_link_container::container_t& ordering
)
{
    const auto count = boost::num_vertices(state.graph());
    std::vector<char> reachable(count, 0);
    const auto root = state.root_node();
    if (root != vertex_descriptor_not_present && root < count)
    {
        std::vector<vertex_descriptor_t> pending{ root };
        while (!pending.empty())
        {
            const auto vd = pending.back();
            pending.pop_back();
            if (vd >= count || reachable[vd])
            {
                continue;
            }
            reachable[vd] = 1;
            for (const auto child : ::subrange(boost::adjacent_vertices(vd, state.graph())))
            {
                pending.push_back(child);
            }
        }
    }

    std::vector<vertex_descriptor_t> held;
    state.aggregates().collect_vertex_descriptors(held);
    state.versions().collect_vertex_descriptors(held);
    statements.collect_vertex_descriptors(held);
    ordering.collect_vertex_descriptors(held);
    hold_vertex(held, root);
    for (const auto vd : state.wizard_statements())
    {
        hold_vertex(held, vd);
    }

    std::vector<char> kept = reachable;
    for (const auto vd : held)
    {
        if (vd < count)
        {
            kept[vd] = 1;
        }
    }
    hold_kept_entities(state, kept, held);
    for (const auto vd : held)
    {
        if (vd < count)
        {
            kept[vd] = 1;
        }
    }
    return kept;
}

void pull_graph(language_part_t& state, const vertex_descriptor_t removed)
{
    auto& vertices = state.graph_container();
    const auto count = boost::num_vertices(state.graph());
    for (vertex_descriptor_t vd = 0; vd < count; ++vd)
    {
        const auto modified = vertices.modify_at(vd, [&](auto& stored)
        {
            auto& property = stored.m_property;
            property.vd_self = vd;
            pull_down(property.vd_parent, removed);
            if (auto* tag = std::get_if<tag_statement_t>(&property.entity))
            {
                for (auto& value : tag->string_values_at_zero_level)
                {
                    pull_down(value.second, removed);
                }
            }
            if (auto* wizard = std::get_if<wizard_statement_t>(&property.entity))
            {
                pull_down(wizard->chunk_node, removed);
            }
        });
        if (!modified)
        {
            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::holder{},
                exception::e_what_info{ "a vertex descriptor collided after a vertex was removed" }
            );
        }
    }
}

}// namespace drop_detached

// vertices the root cannot reach and that no index stores are erased.
// descriptors above each erased vertex move down by one
//
void language_part_t::drop_detached_vertices()
{
    const auto kept = drop_detached::vertices_to_keep(*this, version_statements_, ordering_links_);
    const auto count = kept.size();
    auto garbage =
        views::iota(vertex_descriptor_t{ 0 }, count)
        | views::filter([&](const vertex_descriptor_t vd)
        {
            return !kept[vd];
        })
        | ranges::to_vector
        ;
    garbage |= actions::sort(greater{});

    for (const auto removed : garbage)
    {
        boost::clear_vertex(removed, graph_);
        // remove_vertex asks the vertex list for iterator stability, which this list does not provide.
        // the dispatch erases the vertex and moves later edge endpoints down by one
        boost::detail::remove_vertex_dispatch(graph_, removed, boost::bidirectional_tag{});
        drop_detached::pull_graph(*this, removed);
        aggregates_.note_vertex_removed(removed);
        versions_.note_vertex_removed(removed);
        version_statements_.note_vertex_removed(removed);
        ordering_links_.note_vertex_removed(removed);
        pull_down(root_node_, removed);

        auto stored = wizard_statements_;
        wizard_statements_ =
            stored
            | views::filter([&](const vertex_descriptor_t vd)
            {
                return vd != removed;
            })
            | views::transform([&](vertex_descriptor_t vd)
            {
                pull_down(vd, removed);
                return vd;
            })
            | ranges::to<vertex_descriptors_t>()
            ;
    }
}

bool language_part_t::version_names_tag(
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index,
    const name_t& name
) const
{
    return version_statements_.names_tag(vd_aggregate, version_index, name);
}

index_t language_part_t::next_version_statement_index(
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
) const
{
    return version_statements_.next_index(vd_aggregate, version_index);
}

vertex_descriptors_t language_part_t::wizard_statements() const
{
    return wizard_statements_;
}

void language_part_t::note_wizard_statement(const vertex_descriptor_t vd)
{
    wizard_statements_.push_back(vd);
}

void language_part_t::refresh_live_and_hashed(const vertex_descriptor_t vd_aggregate)
{
    index_ot first;
    for (const auto& declaration : versions_.declarations_in_order(vd_aggregate))
    {
        if (declaration.status)
        {
            continue;
        }
        if (!first || declaration.index < *first)
        {
            first = declaration.index;
        }
    }
    aggregates_.note_first_live(vd_aggregate, first);
    aggregates_.note_last_hashed(vd_aggregate, versions_.last_hashed_index(vd_aggregate));
}

std::vector<version_statement_container::container_t::statement_t> language_part_t::statements_of(
    const vertex_descriptor_t vd_definition
) const
{
    return version_statements_.of_definition(vd_definition);
}

bool language_part_t::is_only_version_delete(const vertex_descriptor_t vd_definition) const
{
    return version_statements_.is_only_version_delete(vd_definition);
}

bool language_part_t::ordering_names(const name_t& name) const
{
    return ordering_links_.names(name);
}

std::vector<version_statement_container::container_t::statement_t> language_part_t::statements_in_order(
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
) const
{
    return version_statements_.in_order(vd_aggregate, version_index);
}

compiled_part_t::compiled_part_t()
{
    fLogmeD("compiled state created");
}

compiled_part_t::~compiled_part_t()
{
    fLogmeD("compiled state destroyed");
}

void compiled_part_t::register_get_static_part_of_label_func(const get_static_part_of_label_ft& func)
{
    get_static_part_of_label_ = func;
}

aggregated_labels_static_ot compiled_part_t::notify_labels_for_tag(const notify_labels_for_tag_t& args)
{
    return tag_labels_container_.find_by_tag_and_do(
        args.vd_tag,
        // found
        [](auto& /* index */, const auto /* it */, const auto& record)
        {
            return record.aggregated_labels_static;
        },
        // not found
        [&](auto& /* index */)
        {
            const auto& vd_labels = args.get_vd_labels();
            const auto [it, _] = tag_labels_container_.insert({
                .vd_tag = args.vd_tag,
                .vd_labels = vd_labels,
                .aggregated_labels_static = get_static_labels(vd_labels)
            });
            return it->aggregated_labels_static;
        }
    );
}

aggregated_labels_static_ot compiled_part_t::get_static_labels_for_tag(const vertex_descriptor_t vd_tag)
{
    return tag_labels_container_.find_by_tag_and_do(
        vd_tag,
        // found
        [](auto& /* index */, const auto /* it */, const auto& record)
        {
            return record.aggregated_labels_static;
        },
        // not found
        [](auto& /* index */) -> aggregated_labels_static_ot
        {
            return {};
        }
    );
}

void compiled_part_t::notify_name_for_tag(const notify_name_for_tag_t& args)
{
    tag_name_container_.insert({
        .vd_tag = args.vd_tag,
        .name = args.name
    });
}

name_t compiled_part_t::get_name_for_tag(const get_name_for_tag_t& args)
{
    if (args.name)
    {
        return args.name.value();
    }
    else{
        return tag_name_container_.find_by_tag(args.vd_tag).name;
    }
}

name_ot compiled_part_t::try_name_for_tag(const vertex_descriptor_t vd_tag) const
{
    return tag_name_container_.try_name_by_tag(vd_tag);
}

void compiled_part_t::notify_ordering_for_version(const notify_ordering_for_version_t& args)
{
    ordering_container_.push_back({
        .version_index = args.version_index,
        .ordering = args.ordering
    });
}

void compiled_part_t::notify_default_aggregates_ordering(const notify_default_aggregates_ordering_t& args)
{
    auto version_indexes =
        ranges::subrange(ordering_container_.begin(), ordering_container_.end())
        | views::transform([](const auto& record){ return record.version_index; })
        | to_vector
        ;

    version_indexes |= actions::sort | actions::unique;

    for (const auto& version_index : version_indexes)
    {
        for (const auto& [_, ordering] : ordering_container_.try_find_range_by_version_index(version_index))
        {
            const auto ranks = ranks_of_ordering(ordering.ordering_parts);
            if (ranks.problem)
            {
                continue;
            }

            index_t after = 0;
            for (const auto& [name, rank] : ranks.rank)
            {
                after = std::max(after, rank + 1);
            }

            // aggregates the ordering does not name keep the default order, after the named ones.
            // equal ranks keep the default order
            auto order_key = ranks.rank;
            std::map<name_t, index_t> default_pos;
            index_t place = 0;
            for (const auto& record : args.default_ordering)
            {
                default_pos.emplace(record.aggregate_name, place++);
                if (order_key.find(record.aggregate_name) == order_key.end())
                {
                    order_key.emplace(record.aggregate_name, after++);
                }
            }

            auto sorting = args.default_ordering;
            const auto key_of = [&](const auto& record)
            {
                return std::pair{ order_key.at(record.aggregate_name), default_pos.at(record.aggregate_name) };
            };
            sorting |= actions::sort(less{}, key_of);

            fLogmeD("ordering: '{}', version: {}", ordering.name, version_index);

            const auto tab_1 = log_tabulator_t();

            for (const auto& [index, record] : views::enumerate(sorting))
            {
                ordering_ranks_.insert({
                    .aggregate_name = record.aggregate_name,
                    .version_index = version_index,
                    .ordering_name = ordering.name,
                    .rank = index
                });
                fLogmeD("index: {}, aggregate: '{}'", index, record.aggregate_name);
            }
        }
    }
}

void compiled_part_t::force_resolve_labels()
{
    const auto unique_vd_labels =
        tag_labels_container_
        | views::filter([](const auto& record){ return record.vd_labels.has_value(); })
        | views::transform([](const auto& record) -> const vertex_descriptors_t& { return record.vd_labels.value(); })
        | views::join
        | to<std::set<vertex_descriptor_t>>
        ;

    for (const auto& vd_label : unique_vd_labels)
    {
        find_or_get_static_label(vd_label);
    }

    // the refs are resolved into static values; the compiled state does not keep them
    std::vector<vertex_descriptor_t> tags;
    for (const auto& record : tag_labels_container_)
    {
        tags.push_back(record.vd_tag);
    }
    for (const auto vd_tag : tags)
    {
        tag_labels_container_.try_find_by_and_modify<tag_labels_container::detail::by_tag>(
            vd_tag,
            [](auto& record)
            {
                record.vd_labels.reset();
            }
        );
    }
}

aggregated_label_static_rt compiled_part_t::find_or_get_static_label(const vertex_descriptor_t& vd_label)
{
    return static_label_container_.find_by_label_and_do(
        vd_label,
        [](auto& /* index */, auto /* it */, const auto& record) -> const aggregated_label_static_t&
        {
            return record.static_part;
        },
        [&](auto& index) -> const aggregated_label_static_t&
        {
            const auto [it, _] = static_label_container_.insert({
                .vd_label = vd_label,
                .static_part = get_static_part_of_label_(vd_label)
            });
            return it->static_part;
        }
    );
}

compiled_part_t::merged_versions_t& compiled_part_t::add_merged(const vertex_descriptor_t vd_aggregate)
{
    auto [it, inserted] = merged_.try_emplace(vd_aggregate);
    (void)inserted;
    it->second.vd_aggregate = vd_aggregate;
    return it->second;
}

compiled_part_t::merged_versions_t& compiled_part_t::merged_of(const vertex_descriptor_t vd_aggregate)
{
    const auto found = merged_.find(vd_aggregate);
    if (found == merged_.end())
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::holder{},
            exception::e_what_info{ "cannot find merged versions" }
        );
    }
    return found->second;
}

const compiled_part_t::merged_versions_t& compiled_part_t::merged_of(const vertex_descriptor_t vd_aggregate) const
{
    const auto found = merged_.find(vd_aggregate);
    if (found == merged_.end())
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::holder{},
            exception::e_what_info{ "cannot find merged versions" }
        );
    }
    return found->second;
}

vertex_descriptor_ot compiled_part_t::try_merged_version(
    const vertex_descriptor_t vd_aggregate,
    const index_t version_index
) const
{
    const auto merged = merged_.find(vd_aggregate);
    if (merged == merged_.end())
    {
        return {};
    }

    const auto version = merged->second.versions.find(version_index);
    if (version == merged->second.versions.end())
    {
        return {};
    }
    return version->second;
}

aggregated_labels_static_ot compiled_part_t::get_static_labels(const vertex_descriptors_ot& vd_labels)
{
    return vd_labels.transform([&](const vertex_descriptors_t& vds) -> aggregated_labels_static_t
    {
        return
            vds
            | views::transform([&](const auto& vd){ return find_or_get_static_label(vd); })
            | to<aggregated_labels_static_t>
            ;
    });
}

void compiled_part_t::remember_rendered_string(
    const vertex_descriptor_t vd_version,
    const vertex_descriptor_t vd_string,
    std::string value
)
{
    rendered_strings_.insert_or_assign(
        std::pair{ vd_version, vd_string },
        std::move(value)
    );
}

const std::string* compiled_part_t::rendered_string(
    const vertex_descriptor_t vd_version,
    const vertex_descriptor_t vd_string
) const
{
    const auto found = rendered_strings_.find(std::pair{ vd_version, vd_string });
    if (found == rendered_strings_.end())
    {
        return nullptr;
    }
    return &found->second;
}

void compiled_part_t::remember_value_before(
    const vertex_descriptor_t vd_statement,
    std::optional<strings_t> value
)
{
    value_before_.insert_or_assign(vd_statement, std::move(value));
}

const std::optional<strings_t>* compiled_part_t::value_before(const vertex_descriptor_t vd_statement) const
{
    const auto found = value_before_.find(vd_statement);
    if (found == value_before_.end())
    {
        return nullptr;
    }
    return &found->second;
}

}// namespace galimatron::translator::state::holder
