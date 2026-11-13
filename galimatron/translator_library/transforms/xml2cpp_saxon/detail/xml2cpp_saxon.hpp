#pragma once

// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/transforms/layout_table.hpp"
#include "galimatron/translator_library/transforms/xml2cpp_saxon/detail/node.hpp"
#include "galimatron/translator_library/xml_tags.hpp"


namespace galimatron::translator::transforms::xml2cpp_saxon
{

using namespace galimatron::translator::state;

namespace tags = gal_and_xml_tags;

class xml2cpp_saxon_t
{
public:
    xml2cpp_saxon_t() = delete;

    explicit xml2cpp_saxon_t(
        node_t root,
        layout::table_t* layout_table
    );

    xml2cpp_saxon_t(const xml2cpp_saxon_t&) = delete;
    xml2cpp_saxon_t& operator=(const xml2cpp_saxon_t&) = delete;
    xml2cpp_saxon_t(xml2cpp_saxon_t&&) = delete;
    xml2cpp_saxon_t& operator=(xml2cpp_saxon_t&&) = delete;

public:
    holder::language_part_sptr_t translate();

private:
    void bind(node_t root);
    location_ot read_location(const node_t&) const;
    string_ot read_cdata(const node_t&) const;
    string_ot attribute_if(const node_t&, std::string_view) const;
    index_t attribute_index(const node_t&) const;
    index_ot attribute_index_if(const node_t&) const;

    // once and local. an absent mode inherits
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::xml2cpp
    //
    tag_mode_et attribute_mode(const node_t&) const;

    version_status_oet attribute_status(const node_t&) const;

    void walk(const node_t&, vertex_descriptor_t parent);

    void read_chunk(const node_t&);
    void read_root_aggregate(const node_t&, vertex_descriptor_t parent);
    void read_root_aggregate_version_declarations(const node_t&, vertex_descriptor_t parent);
    void read_root_aggregate_version_declaration(const node_t&, vertex_descriptor_t parent);
    void read_root_aggregate_version_definitions(const node_t&, vertex_descriptor_t parent);
    void read_root_aggregate_version_definition(const node_t&, vertex_descriptor_t parent);

    void read_aggregate(const node_t&, vertex_descriptor_t parent);
    void read_aggregate_version_declarations(const node_t&, vertex_descriptor_t parent);
    void read_aggregate_version_declaration(const node_t&, vertex_descriptor_t parent);
    void read_aggregate_version_definitions(const node_t&, vertex_descriptor_t parent);
    void read_aggregate_version_definition(const node_t&, vertex_descriptor_t parent);

    void read_tag_statement(const node_t&, vertex_descriptor_t parent);
    void read_tag_values(const node_t&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_value(const node_t&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_record(const node_t&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_keyvalue(const node_t&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_array(const node_t&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_string(const node_t&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag, index_ot value_index);

    void read_label_statement(const node_t&, vertex_descriptor_t parent);
    void read_ordering_statement(const node_t&, vertex_descriptor_t parent);
    void read_operation_statement(const node_t&, vertex_descriptor_t parent);

    void unexpected(const node_t&) const;
    void restore_layout(const node_t&, vertex_descriptor_t);
    void read_string_delimiters(const node_t&);
    index_t attribute_delimiter(const node_t&) const;

private:
    layout::table_t* layout_table_ = nullptr;
    holder::language_part_sptr_t state_;
    builder::builder_t builder_;
    index_t next_aggregate_index_ = 0;

    // destroyed after source_, so a child wrapper dies before the document
    node_t kept_document_;
    node_t anchor_;
    node_t source_;
};

}// namespace galimatron::translator::transforms::xml2cpp_saxon
