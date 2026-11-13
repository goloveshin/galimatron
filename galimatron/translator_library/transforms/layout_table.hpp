#pragma once

// galimatron
#include "galimatron/translator_library/pugi_forward.hpp"

// global
#include "global/common_library/type_declaration.hpp"


namespace galimatron::translator::layout
{

struct entry_t
{
    content_ot user_formatting;
    strings_t user_comments;
    std::string fingerprint;
};

struct table_t
{
    std::uint64_t next = 0;
    std::unordered_map<std::uint64_t, entry_t> entries;

    void clear()
    {
        next = 0;
        entries.clear();
    }
};

// the node's names, attributes, and text, without the layout mark
// whitespace the serializer inserts between nodes is ignored
//
std::string fingerprint(const pugi::xml_node& node);

}// namespace galimatron::translator::layout
