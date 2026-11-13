#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::state::builder
{

class builder_t
{
public:
    builder_t(holder::language_part_t&);

    // the state already contains root: new vertices are added to it
    struct on_existing_state_t {};
    builder_t(holder::language_part_t&, on_existing_state_t);

    vertex_descriptor_t root_node() const { return state_.root_node(); }

    holder::language_part_t& state() { return state_; }

    holder::vertex_graph::graph_t& graph() const { return graph_; };

public:
    //--------------------------------------------------------------------------
    // Root
    //--------------------------------------------------------------------------

    struct create_root_t
    {
    };
    vertex_descriptor_t create_root(const create_root_t&);

    //--------------------------------------------------------------------------
    // Chunk
    //--------------------------------------------------------------------------

    struct create_chunk_t
    {
        vertex_descriptor_t root_node;
        location_ot location;
    };
    vertex_descriptor_t create_chunk(const create_chunk_t&);

    //--------------------------------------------------------------------------
    // Root Aggregate
    //--------------------------------------------------------------------------

    struct create_root_aggregate_t
    {
        vertex_descriptor_t chunk_node;
        name_t name;
        location_ot location;
        bool created_by_wizard = false;
    };
    vertex_descriptor_t create_root_aggregate(const create_root_aggregate_t&);

    struct create_root_aggregate_version_declarations_t
    {
        vertex_descriptor_t root_aggregate_node;
    };
    vertex_descriptor_t create_root_aggregate_version_declarations(const create_root_aggregate_version_declarations_t&);

    struct create_root_aggregate_version_declaration_t
    {
        vertex_descriptor_t versions_node;
        index_t index;
        version_status_oet status;
        string_ot comment;
        string_ot hash;
    };
    vertex_descriptor_t create_root_aggregate_version_declaration(const create_root_aggregate_version_declaration_t&);

    struct create_root_aggregate_version_definitions_t
    {
        vertex_descriptor_t root_aggregate_node;
    };
    vertex_descriptor_t create_root_aggregate_version_definitions(const create_root_aggregate_version_definitions_t&);

    struct create_root_aggregate_version_definition_t
    {
        vertex_descriptor_t versions_node;
        index_ot index;
        // set when this is a parked body: version before N
        index_ot owner;
        bool created_by_wizard = false;
        tag_mode_et mode = tag_mode_et::INHERIT;
    };
    vertex_descriptor_t create_root_aggregate_version_definition(const create_root_aggregate_version_definition_t&);

    //--------------------------------------------------------------------------
    // Aggregate
    //--------------------------------------------------------------------------

    struct create_aggregate_t
    {
        vertex_descriptor_t chunk_node;
        name_t name;
        index_t index;
        location_ot location;
        bool created_by_wizard = false;
    };
    vertex_descriptor_t create_aggregate(const create_aggregate_t&);

    struct create_aggregate_version_declarations_t
    {
        vertex_descriptor_t aggregate_node;
    };
    vertex_descriptor_t create_aggregate_version_declarations(const create_aggregate_version_declarations_t&);

    struct create_aggregate_version_declaration_t
    {
        vertex_descriptor_t versions_node;
        index_t index;
        version_status_oet status;
        string_ot comment;
        string_ot hash;
    };
    vertex_descriptor_t create_aggregate_version_declaration(const create_aggregate_version_declaration_t&);

    struct create_aggregate_version_definitions_t
    {
        vertex_descriptor_t aggregate_node;
    };
    vertex_descriptor_t create_aggregate_version_definitions(const create_aggregate_version_definitions_t&);

    struct create_aggregate_version_definition_t
    {
        vertex_descriptor_t versions_node;
        index_ot index;
        // set when this is a parked body: version before N
        index_ot owner;
        bool created_by_wizard = false;
        tag_mode_et mode = tag_mode_et::INHERIT;
    };
    vertex_descriptor_t create_aggregate_version_definition(const create_aggregate_version_definition_t&);

    //--------------------------------------------------------------------------
    // Tag
    //--------------------------------------------------------------------------

    struct create_tag_statement_t
    {
        vertex_descriptor_t version_definition_node;
        index_t index_in_version;
        name_ot name;
        std::deque<name_t> labels;
        tag_mode_et mode = tag_mode_et::INHERIT;
        location_ot location;
    };
    vertex_descriptor_t create_tag_statement(const create_tag_statement_t&);

    struct create_tag_values_t
    {
        vertex_descriptor_t parent_node;
    };
    vertex_descriptor_t create_tag_values(const create_tag_values_t&);

    struct create_tag_value_t
    {
        vertex_descriptor_t parent_node;
        index_ot index;
    };
    vertex_descriptor_t create_tag_value(const create_tag_value_t&);

    struct create_tag_record_t
    {
        vertex_descriptor_t parent_node;
    };
    vertex_descriptor_t create_tag_record(const create_tag_record_t&);

    struct create_tag_keyvalue_t
    {
        vertex_descriptor_t parent_node;
        string_ot key;
        index_t key_delimiter = 0;
    };
    vertex_descriptor_t create_tag_keyvalue(const create_tag_keyvalue_t&);

    struct create_tag_array_t
    {
        vertex_descriptor_t parent_node;
    };
    vertex_descriptor_t create_tag_array(const create_tag_array_t&);

    struct create_tag_string_t
    {
        vertex_descriptor_t parent_node;
        std::string value;
        index_t delimiter = 0;
        bool is_template = false;
    };
    vertex_descriptor_t create_tag_string(const create_tag_string_t&);

    //--------------------------------------------------------------------------
    // Label
    //--------------------------------------------------------------------------

    struct create_label_statement_t
    {
        vertex_descriptor_t version_definition_node;
        index_t index;
        name_t name;
        location_ot location;
    };
    vertex_descriptor_t create_label_statement(const create_label_statement_t&);

    struct create_label_dynamic_t
    {
        vertex_descriptor_t label_node;
        std::string value;
    };
    vertex_descriptor_t create_label_dynamic(const create_label_dynamic_t&);

    struct create_label_static_t
    {
        vertex_descriptor_t label_node;
        strings_t values;
        string_ot tag_name_from;
    };
    vertex_descriptor_t create_label_static(const create_label_static_t&);

    //--------------------------------------------------------------------------
    // Ordering
    //--------------------------------------------------------------------------

    struct create_ordering_statement_t
    {
        vertex_descriptor_t version_definition_node;
        index_t index;
        name_t name;

        using ordering_part_t = std::deque<name_t>;
        using ordering_parts_t = std::deque<ordering_part_t>;
        ordering_parts_t ordering_parts;

        location_ot location;
    };
    vertex_descriptor_t create_ordering_statement(const create_ordering_statement_t&);

    //--------------------------------------------------------------------------
    // Wizard
    //--------------------------------------------------------------------------

    struct create_wizard_statement_t
    {
        vertex_descriptor_t place_node;
        wizard::command_et type;
        wizard::context_et context;
        vertex_descriptor_t chunk_node;
        path_t chunk_uri;
        wizard::command_data_t data;
        std::vector<wizard::batch_command_t> commands = {};
    };
    vertex_descriptor_t create_wizard_statement(const create_wizard_statement_t&);

    //--------------------------------------------------------------------------
    // Operation
    //--------------------------------------------------------------------------

    struct create_operation_statement_t
    {
        vertex_descriptor_t version_definition_node;
        index_t index;
        name_t name;
        bool is_version = false;
        location_ot location;
    };
    vertex_descriptor_t create_operation_statement(const create_operation_statement_t&);

    // delete; in an aggregate version definition
    //
    void apply_aggregate_version_delete(vertex_descriptor_t vd_definition);

    // copy a vertex and its children under a new parent.
    // a tag statement receives tag_index when it is set
    //
    struct clone_under_t
    {
        vertex_descriptor_t source;
        vertex_descriptor_t parent;
        index_ot tag_index;
        bool created_by_wizard = false;
    };
    vertex_descriptor_t clone_under(const clone_under_t&);

    // copy a vertex and its children from another graph under a new parent.
    // tag_index replaces the tag's index when set.
    // created_by_wizard replaces the tag flag when set.
    // clear_formatting drops source text on the copy and its children
    //
    struct clone_from_t
    {
        const holder::language_part_t::graph_t& source_graph;
        vertex_descriptor_t source;
        vertex_descriptor_t parent;
        index_ot tag_index;
        bool_ot created_by_wizard;
        bool clear_formatting = false;
    };
    vertex_descriptor_t clone_from(const clone_from_t&);

private:
    struct clone_under_impl_t
    {
        vertex_descriptor_t source;
        vertex_descriptor_t parent;
        index_ot tag_index;
        bool created_by_wizard = false;
        const std::map<vertex_descriptor_t, index_t>* string_sources;
        std::map<index_t, vertex_descriptor_t>* string_targets;
    };
    vertex_descriptor_t clone_under_impl(const clone_under_impl_t&);

    struct clone_from_impl_t
    {
        const holder::language_part_t::graph_t* source_graph;
        vertex_descriptor_t source;
        vertex_descriptor_t parent;
        index_ot tag_index;
        bool_ot created_by_wizard;
        bool clear_formatting = false;
        const std::map<vertex_descriptor_t, index_t>* string_sources;
        std::map<index_t, vertex_descriptor_t>* string_targets;
    };
    vertex_descriptor_t clone_from_impl(const clone_from_impl_t&);

    vertex_descriptor_t add_vertex(const holder::vertex_t&);
    path_ot chunk_path(vertex_descriptor_t chunk_node) const;

private:
    holder::language_part_t& state_;
    holder::vertex_graph::graph_t& graph_;
};

}// namespace galimatron::translator::state::builder
