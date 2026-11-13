#pragma once

// galimatron
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/xml_tags.hpp"
#include "galimatron/translator_library/transforms/cpp2xml/cpp2xml.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"


namespace galimatron::translator::transforms::cpp2xml
{

using namespace galimatron::translator::state;

using namespace galimatron::translator::state::holder;

class cpp2xml_t
{
public:
    cpp2xml_t() = delete;
    cpp2xml_t(const translate_t&);
    cpp2xml_t(const cpp2xml_t&) = delete;
    cpp2xml_t& operator=(const cpp2xml_t&) = delete;
    cpp2xml_t(cpp2xml_t&&) = delete;
    cpp2xml_t& operator=(cpp2xml_t&&) = delete;

public:
    pugi_document_with_meanful_node_t translate();

private:

    //--------------------------------------------------------------------------
    // Root
    //--------------------------------------------------------------------------

    pugi::xml_node create_root(const root_t&);

    //--------------------------------------------------------------------------
    // Chunk
    //--------------------------------------------------------------------------

    pugi::xml_node create_chunk(const chunk_t&);

    //--------------------------------------------------------------------------
    // Root Aggregate
    //--------------------------------------------------------------------------

    pugi::xml_node create_root_aggregate(const root_aggregate_t&);
    pugi::xml_node create_root_aggregate_version_declarations(const root_aggregate_version_declarations_t&);
    pugi::xml_node create_root_aggregate_version_declaration(const root_aggregate_version_declaration_t&);
    pugi::xml_node create_root_aggregate_version_definitions(const root_aggregate_version_definitions_t&);
    pugi::xml_node create_root_aggregate_version_definition(const root_aggregate_version_definition_t&);

    //--------------------------------------------------------------------------
    // Aggregate
    //--------------------------------------------------------------------------

    pugi::xml_node create_aggregate(const aggregate_t&);
    pugi::xml_node create_aggregate_version_declarations(const aggregate_version_declarations_t&);
    pugi::xml_node create_aggregate_version_declaration(const aggregate_version_declaration_t&);
    pugi::xml_node create_aggregate_version_definitions(const aggregate_version_definitions_t&);
    pugi::xml_node create_aggregate_version_definition(const aggregate_version_definition_t&);

    //--------------------------------------------------------------------------
    // Tag
    //--------------------------------------------------------------------------

    pugi::xml_node create_tag_statement(const tag_statement_t&);
    pugi::xml_node create_tag_values(const tag_values_t&);
    pugi::xml_node create_tag_value(const tag_value_t&);
    pugi::xml_node create_tag_record(const tag_record_t&);
    pugi::xml_node create_tag_keyvalue(const tag_keyvalue_t&);
    pugi::xml_node create_tag_array(const tag_array_t&);
    pugi::xml_node create_tag_string(const tag_string_t&);

    //--------------------------------------------------------------------------
    // Label
    //--------------------------------------------------------------------------

    pugi::xml_node create_label_statement(const label_statement_t&);
    pugi::xml_node create_label_dynamic(const label_dynamic_t&);
    pugi::xml_node create_label_static(const label_static_t&);

    //--------------------------------------------------------------------------
    // Ordering
    //--------------------------------------------------------------------------

    pugi::xml_node create_ordering_statement(const ordering_statement_t&);

    //--------------------------------------------------------------------------
    // Operation
    //--------------------------------------------------------------------------

    pugi::xml_node create_operation_statement(const operation_statement_t&);

private:
    // language domain: the wizard created this aggregate or version
    //
    void append_wizard_mark(pugi::xml_node, std::string_view);

    pugi::xml_node push_parent(const pugi::xml_node&);
    void pop_parent();
    pugi::xml_node parent() const;
    const vertex_t& vertex();
    void remember_layout(const vertex_t&, pugi::xml_node);
    void write_string_delimiters();

    // the vertices printed under this node.
    // a version lists its statements. a compiled version lists the tags compiled for it.
    // every other node lists its graph edges.
    // a wizard statement is never printed. a compiled print drops labels
    //
    vertex_descriptors_t childs_of(vertex_descriptor_t) const;

private:
    const translate_t args_;
    const holder::language_part_sptr_t& language_state_;
    const holder::compiled_part_sptr_ot& compiled_state_;

    pugi_document_sptr_t doc_;
    pugi_xml_node_ot mode_node_;
    vertex_ocrt vertex_;
    holder::aggregated_labels_static_ot static_labels_;// references to the static labels of the current tag
    vertex_descriptor_ot vd_compiled_version_;// the compiled version whose tags are being printed
    name_ot aggregate_name_;

    std::stack<pugi::xml_node> parent_nodes_;
};

}// namespace galimatron::translator::transforms::cpp2xml
