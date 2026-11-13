#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;

class cpp2post_cpp_t
{
public:
    struct config_t
    {
        holder::language_part_sptr_t language_state;
    };
    cpp2post_cpp_t(const config_t&);

public:
    holder::compiled_part_sptr_t translate();

private:
    using graph_t = holder::language_part_t::graph_t;
    using graph_container_t = holder::language_part_t::graph_container_t;

private:
    using merged_versions_t = holder::compiled_part_t::merged_versions_t;

    struct append_own_version_entities_t
    {
        const holder::language_part_t& language;
        vertex_descriptor_t vd_version;
        index_t version_index;
        index_t& index_in_versions;
        holder::version_entity_container::container_t& versions_entities;
        // statements parked from collapsed versions. once and local are not visible here
        bool from_anonymous = false;
    };
    static void append_own_version_entities(const append_own_version_entities_t&);

    struct append_other_version_entities_t
    {
        const graph_container_t& graph_container;
        const holder::version_entity_container::container_t& versions_entities_other;
        index_t version_index;
        index_t& index_in_versions;
        holder::version_entity_container::container_t& versions_entities;
    };
    static void append_other_version_entities(const append_other_version_entities_t&);

    struct resolve_tag_name_labels_to_vd_labels_t
    {
        const holder::version_entity_container::container_t& versions_entities;
        const graph_t& graph;
        vertex_descriptor_t vd_aggregate;
        tag_mode_et version_mode = tag_mode_et::INHERIT;
        index_t tag_index_in_versions;
        const strings_t& tag_labels;
        index_t version_index;
        path_ot file_path;
    };
    static vertex_descriptors_ot resolve_tag_name_labels_to_vd_labels(const resolve_tag_name_labels_to_vd_labels_t&);

    struct name_tag_from_label_t
    {
        const graph_t& graph;
        const holder::aggregated_labels_static_ot& aggregated_labels_static;
        vertex_descriptor_t vd_tag;
        const tag_statement_t& tag;
        using notify_name_for_tag_ft = std::function <void (const vertex_descriptor_t /* vd_tag */, const name_t&)>;
        const notify_name_for_tag_ft& notify_name_for_tag;
    };
    static void name_tag_from_label(const name_tag_from_label_t&);

    // a tag without a name takes it from a label's tag-name-from
    //
    struct naming_unnamed_tags_t
    {
        const graph_t& graph;
        holder::compiled_part_t& compiled_state;
        vertex_descriptor_t vd_aggregate;
    };
    static void naming_unnamed_tags(const naming_unnamed_tags_t&);

    struct create_merged_versions_entities_t
    {
        const holder::language_part_t& language;
        const graph_container_t& graph_container;
        const holder::version_container::container_t& versions;
        vertex_descriptor_t vd_aggregate;
        const merged_versions_t* root_merged;
        holder::compiled_part_t& compiled_state;
    };
    static merged_versions_t& create_merged_versions_entities(const create_merged_versions_entities_t&);

    struct compile_merged_versions_entities_t
    {
        const graph_t& graph;
        const graph_container_t& graph_container;
        holder::compiled_part_t& compiled_state;
        vertex_descriptor_t vd_aggregate;
        bool is_root_aggregate;
        const holder::language_part_t& language;
    };
    static void compile_merged_versions_entities(const compile_merged_versions_entities_t&);

    struct check_labels_in_version_t
    {
        const graph_t& graph;
        const graph_container_t& graph_container;
        holder::compiled_part_t& compiled_state;
        vertex_descriptor_t vd_aggregate;
        index_t version_index;
    };
    static void check_labels_in_version(const check_labels_in_version_t&);

    struct compile_tags_in_version_t
    {
        const graph_t& graph;
        holder::compiled_part_t& compiled_state;
        vertex_descriptor_t vd_aggregate;
        index_t version_index;
        std::function<void (const std::string&)> log_func;
    };
    static vertex_descriptors_t compile_tags_in_version(const compile_tags_in_version_t&);

    struct compile_orderings_in_version_t
    {
        const graph_t& graph;
        holder::compiled_part_t& compiled_state;
        vertex_descriptor_t vd_aggregate;
        index_t version_index;
        const holder::language_part_t& language;
    };
    static void compile_orderings_in_version(const compile_orderings_in_version_t&);

    struct check_ordering_names_t
    {
        const graph_t& graph;
        const holder::language_part_t& language;
        vertex_descriptor_t vd_ordering;
        index_t version_index;
        const ordering_statement_t& ordering;
    };
    static void check_ordering_names(const check_ordering_names_t&);

private:
    const config_t config_;
    const graph_t& graph_;
    const graph_container_t& graph_container_;
};

}// namespace galimatron::translator::transforms::cpp2post_cpp
