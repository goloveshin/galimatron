#pragma once

// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/transforms/xml2cpp/xml2cpp.hpp"
#include "galimatron/translator_library/xml_tags.hpp"


namespace galimatron::translator::transforms::xml2cpp
{

using namespace galimatron::translator::state;

namespace tags = gal_and_xml_tags;

class xml2cpp_t
{
public:
    xml2cpp_t() = delete;

    explicit xml2cpp_t(const translate_t&);

    xml2cpp_t(const xml2cpp_t&) = delete;
    xml2cpp_t& operator=(const xml2cpp_t&) = delete;
    xml2cpp_t(xml2cpp_t&&) = delete;
    xml2cpp_t& operator=(xml2cpp_t&&) = delete;

public:
    holder::language_part_sptr_t translate();

private:
    pugi::xml_node source_node() const;
    location_ot read_location(const pugi::xml_node&) const;
    string_ot read_cdata(const pugi::xml_node&) const;
    string_ot attribute_if(const pugi::xml_node&, std::string_view) const;
    index_t attribute_index(const pugi::xml_node&) const;
    index_ot attribute_index_if(const pugi::xml_node&) const;

    // once and local. an absent mode inherits
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::xml2cpp
    //
    tag_mode_et attribute_mode(const pugi::xml_node&) const;

    version_status_oet attribute_status(const pugi::xml_node&) const;

    void walk(const pugi::xml_node&, vertex_descriptor_t parent);

    void read_chunk(const pugi::xml_node&);
    void read_root_aggregate(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_root_aggregate_version_declarations(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_root_aggregate_version_declaration(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_root_aggregate_version_definitions(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_root_aggregate_version_definition(const pugi::xml_node&, vertex_descriptor_t parent);

    void read_aggregate(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_aggregate_version_declarations(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_aggregate_version_declaration(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_aggregate_version_definitions(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_aggregate_version_definition(const pugi::xml_node&, vertex_descriptor_t parent);

    void read_tag_statement(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_tag_values(const pugi::xml_node&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_value(const pugi::xml_node&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_record(const pugi::xml_node&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_keyvalue(const pugi::xml_node&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_array(const pugi::xml_node&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag);
    void read_tag_string(const pugi::xml_node&, vertex_descriptor_t parent, vertex_descriptor_t vd_tag, index_ot value_index);

    void read_label_statement(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_ordering_statement(const pugi::xml_node&, vertex_descriptor_t parent);
    void read_operation_statement(const pugi::xml_node&, vertex_descriptor_t parent);

    void unexpected(const pugi::xml_node&) const;
    void restore_layout(const pugi::xml_node&, vertex_descriptor_t);
    void read_string_delimiters(const pugi::xml_node&);
    index_t attribute_delimiter(const pugi::xml_node&) const;

private:
    const translate_t args_;
    holder::language_part_sptr_t state_;
    builder::builder_t builder_;
    index_t next_aggregate_index_ = 0;
};

}// namespace galimatron::translator::transforms::xml2cpp
