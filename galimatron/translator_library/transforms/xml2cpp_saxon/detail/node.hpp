#pragma once

// galimatron
#include "galimatron/translator_library/transforms/layout_table.hpp"


class XdmNode;

namespace galimatron::translator::transforms::xml2cpp_saxon
{

// an XdmNode the walk owns, or an empty node.
// an empty node is false, and child() on it is empty
//
// testing: no
//
class node_t
{
public:
    struct attribute_t
    {
        bool present = false;
        std::string text;

        explicit operator bool() const
        {
            return present;
        }

        const char* value() const
        {
            return text.c_str();
        }
    };

    struct attribute_record_t
    {
        std::string name;
        std::string value;
    };

public:
    node_t() = default;
    ~node_t();

    node_t(node_t&& other) noexcept;
    node_t& operator=(node_t&& other) noexcept;

    node_t(const node_t&) = delete;
    node_t& operator=(const node_t&) = delete;

    explicit operator bool() const;

    std::string_view local_name() const;
    bool is_element() const;
    bool is_text() const;
    std::string text() const;

    node_t parent() const;
    node_t child(std::string_view name) const;
    std::vector<node_t> children() const;
    std::vector<node_t> children(std::string_view name) const;
    attribute_t attribute(std::string_view name) const;
    std::vector<attribute_record_t> attributes() const;

    // owns the node when owns is true. a null node is empty
    //
    static node_t adopt(XdmNode* node, bool owns);

private:
    XdmNode* node_ = nullptr;
    bool owns_ = false;
};

// the layout marks of the elements under the node, and of the node, whose fingerprint
// equals the one cpp2xml stored for that mark.
// a fingerprint is the names, attributes, and text of an element and its children, without
// the layout mark. whitespace the serializer inserts between nodes is ignored.
// one walk over the tree: each element is read once, however many marked parents it has
//
// testing: no
//
layout::layout_marks_t unchanged_layout_marks(const node_t& root, const layout::table_t& table);

}// namespace galimatron::translator::transforms::xml2cpp_saxon
