#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/graph/graph.hpp"
#include "galimatron/translator_library/state/holder/containers/static_label_container.hpp"
#include "galimatron/translator_library/state/holder/containers/tag_container.hpp"
#include "galimatron/translator_library/state/holder/containers/tag_labels_container.hpp"
#include "galimatron/translator_library/state/holder/containers/tag_name_container.hpp"
#include "galimatron/translator_library/state/holder/containers/ordering_container.hpp"
#include "galimatron/translator_library/state/holder/containers/ordering_rank_container.hpp"
#include "galimatron/translator_library/state/holder/containers/ordering_link_container.hpp"
#include "galimatron/translator_library/state/holder/containers/aggregate_container.hpp"
#include "galimatron/translator_library/state/holder/containers/version_container.hpp"
#include "galimatron/translator_library/state/holder/containers/version_entity_container.hpp"
#include "galimatron/translator_library/state/holder/containers/version_statement_container.hpp"
#include "galimatron/translator_library/state/holder/forward.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/translator/forward.hpp"


namespace galimatron::translator::state::holder
{

class language_part_t
{
public:
    using graph_t = vertex_graph::graph_t;
    using graph_container_t = decltype(graph_t{}.m_vertices);

public:
    language_part_t();
    ~language_part_t();

    language_part_t(const language_part_t&) = delete;
    language_part_t& operator=(const language_part_t&) = delete;
    language_part_t(language_part_t&&) = delete;
    language_part_t& operator=(language_part_t&&) = delete;

public:
    auto& graph(){ return graph_; }
    const auto& graph() const { return graph_; }
    auto root_node() const { return root_node_; }
    void set_root_node(const vertex_descriptor_t& root_node){ root_node_ = root_node; }

    // vertices the root cannot reach and that no index stores are erased.
    // descriptors above each erased vertex move down by one
    //
    // testing: no
    //
    void drop_detached_vertices();
    auto& graph_container(){ return graph_container_; }
    const auto& graph_container() const { return graph_container_; }

    vertex_descriptors_t wizard_statements() const;

    // recorded when the wizard statement vertex is created
    //
    void note_wizard_statement(vertex_descriptor_t vd);

    auto& aggregates(){ return aggregates_; }
    const auto& aggregates() const { return aggregates_; }

    auto& versions(){ return versions_; }
    const auto& versions() const { return versions_; }

    // stores each version definition's child text, and records those definitions' direct statements
    //
    void refresh_version_records();

    // an anonymous body that does not name an owner yet takes the next numbered version.
    // a body that already names its owner keeps that owner
    //
    void bind_anonymous_owners(vertex_descriptor_t vd_definitions);

    // definition links are rebuilt from the child order. declarations stay.
    // an anonymous body is owned by the next numbered version
    //
    void rebind_version_definitions(vertex_descriptor_t vd_aggregate);

    // records a direct child of a version definition. another parent is ignored
    //
    void note_version_child(vertex_descriptor_t vd_child);

    // a move drops the statement here, then note_version_child records the new version
    //
    void forget_version_child(vertex_descriptor_t vd_child);

    // the vertex is unlinked and leaves the version index.
    // it stays until drop_detached_vertices, so descriptors do not move while a command runs
    //
    // testing: no
    //
    void detach_vertex(vertex_descriptor_t vd_parent, vertex_descriptor_t vd_child);

    // every child is detached
    //
    // testing: no
    //
    void detach_children(vertex_descriptor_t vd_parent);

    bool version_names_tag(
        vertex_descriptor_t vd_aggregate,
        index_t version_index,
        const name_t& name
    ) const;

    index_t next_version_statement_index(vertex_descriptor_t vd_aggregate, index_t version_index) const;

    // direct statements of one version, in index order
    //
    std::vector<version_statement_container::container_t::statement_t> statements_in_order(
        vertex_descriptor_t vd_aggregate,
        index_t version_index
    ) const;

    // direct statements of one version body, numbered or anonymous, in index order
    //
    std::vector<version_statement_container::container_t::statement_t> statements_of(
        vertex_descriptor_t vd_definition
    ) const;

    // the body is one statement, and that statement is delete;
    //
    bool is_only_version_delete(vertex_descriptor_t vd_definition) const;

    // a root ordering names this aggregate, compared without case
    //
    bool ordering_names(const name_t& name) const;

    // first-seen string delimiters. index 0 is a plain double quote
    //
    const std::vector<string_delimiter_t>& string_delimiters() const { return string_delimiters_; }
    index_t intern_string_delimiter(const string_delimiter_t&);
    void replace_string_delimiters(std::vector<string_delimiter_t>);

private:
    // smallest live declaration index, and the highest hashed declaration, stored on the aggregate record
    //
    void refresh_live_and_hashed(vertex_descriptor_t vd_aggregate);

    graph_t graph_;
    graph_container_t& graph_container_;
    vertex_descriptor_t root_node_;
    aggregate_container::container_t aggregates_;
    version_container::container_t versions_;
    version_statement_container::container_t version_statements_;
    ordering_link_container::container_t ordering_links_;
    vertex_descriptors_t wizard_statements_;
    std::vector<string_delimiter_t> string_delimiters_;
};
using language_part_sptr_t = std::shared_ptr<language_part_t>;


class compiled_part_t
{
public:
    compiled_part_t();
    ~compiled_part_t();

    compiled_part_t(const compiled_part_t&) = delete;
    compiled_part_t& operator=(const compiled_part_t&) = delete;

public:
    auto& tag_container(){ return tag_container_; }
    const auto& tag_container() const { return tag_container_; }

    auto& tag_labels_container(){ return tag_labels_container_; }
    const auto& tag_labels_container() const { return tag_labels_container_; }

    const auto& ordering_ranks() const { return ordering_ranks_; }

    // get the static info of a label
    using get_static_part_of_label_ft = std::function<aggregated_label_static_t (const vertex_descriptor_t /* vd_label */)>;
    void register_get_static_part_of_label_func(const get_static_part_of_label_ft&);

    struct notify_labels_for_tag_t
    {
        vertex_descriptor_t vd_tag;
        using get_vd_labels_ft = std::function<vertex_descriptors_ot ()>;
        get_vd_labels_ft get_vd_labels;
    };
    aggregated_labels_static_ot notify_labels_for_tag(const notify_labels_for_tag_t&);

    aggregated_labels_static_ot get_static_labels_for_tag(const vertex_descriptor_t);

    struct notify_name_for_tag_t
    {
        vertex_descriptor_t vd_tag;
        name_t name;
    };
    void notify_name_for_tag(const notify_name_for_tag_t&);

    struct get_name_for_tag_t
    {
        vertex_descriptor_t vd_tag;
        name_ot name;
    };
    name_t get_name_for_tag(const get_name_for_tag_t&);

    // the name compilation stored for an unnamed tag. empty when this tag was not compiled
    //
    name_ot try_name_for_tag(vertex_descriptor_t vd_tag) const;

    struct notify_ordering_for_version_t
    {
        index_t version_index;
        const ordering_statement_t& ordering;
    };
    void notify_ordering_for_version(const notify_ordering_for_version_t&);

    struct aggregate_ordering_t
    {
        name_t aggregate_name;
        index_t index;
    };
    using aggregate_orderings_t = std::deque<aggregate_ordering_t>;

    struct notify_default_aggregates_ordering_t
    {
        aggregate_orderings_t default_ordering;
    };
    void notify_default_aggregates_ordering(const notify_default_aggregates_ordering_t&);

    void force_resolve_labels();

    struct merged_versions_t
    {
        vertex_descriptor_t vd_aggregate = {};
        version_entity_container::container_t entities;
        // compiled version body by version index. a deleted version that was not compiled is absent
        std::map<index_t, vertex_descriptor_t> versions;
    };

    merged_versions_t& add_merged(const vertex_descriptor_t vd_aggregate);
    merged_versions_t& merged_of(const vertex_descriptor_t vd_aggregate);
    const merged_versions_t& merged_of(const vertex_descriptor_t vd_aggregate) const;

    // the compiled body of this aggregate version. empty when that version was not compiled
    //
    vertex_descriptor_ot try_merged_version(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index
    ) const;

    // the compiled text of one _te string in one version. the language string stays as written
    //
    void remember_rendered_string(
        const vertex_descriptor_t vd_version,
        const vertex_descriptor_t vd_string,
        std::string value
    );
    const std::string* rendered_string(
        const vertex_descriptor_t vd_version,
        const vertex_descriptor_t vd_string
    ) const;

    // strings visible just before this statement. an empty optional means the tag was absent.
    // missing means this statement was not compiled
    //
    void remember_value_before(
        const vertex_descriptor_t vd_statement,
        std::optional<strings_t> value
    );
    const std::optional<strings_t>* value_before(const vertex_descriptor_t vd_statement) const;

private:
    aggregated_label_static_rt find_or_get_static_label(const vertex_descriptor_t& /* vd_label */);
    aggregated_labels_static_ot get_static_labels(const vertex_descriptors_ot&);

private:
    tag_container::container_t tag_container_;
    tag_labels_container::container_t tag_labels_container_;
    tag_name_container::container_t tag_name_container_;
    static_label_container::container_t static_label_container_;
    ordering_container::container_t ordering_container_;
    ordering_rank_container::container_t ordering_ranks_;
    std::map<vertex_descriptor_t, merged_versions_t> merged_;
    std::map<std::pair<vertex_descriptor_t, vertex_descriptor_t>, std::string> rendered_strings_;
    std::map<vertex_descriptor_t, std::optional<strings_t>> value_before_;

    get_static_part_of_label_ft get_static_part_of_label_;
};
using compiled_part_sptr_t = std::shared_ptr<compiled_part_t>;
using compiled_part_sptr_ot = std::optional<compiled_part_sptr_t>;

}// namespace galimatron::translator::state::holder
