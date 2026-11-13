#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"


namespace galimatron::translator
{

using pugi_document_sptr_t = std::shared_ptr<pugi::xml_document>;
using pugi_xml_node_ot = std::optional<pugi::xml_node>;

struct pugi_document_with_meanful_node_t
{
    pugi_document_sptr_t document;
    pugi_xml_node_ot meanful_node;
};

struct append_location_t
{
    pugi::xml_node& parent_node;
    location_ot location;
    gf::split_path_by_parts_ft split_path_by_parts;
};
using append_location_ot = std::optional<append_location_t>;
void append_location(const append_location_t&);

struct append_path_by_parts_t
{
    pugi::xml_node& parent_node;
    gf::path_by_parts_ot parts;
};
void append_path_by_parts(const append_path_by_parts_t&);

file_content_t printout(const pugi::xml_document&);

}//namespace galimatron::translator
