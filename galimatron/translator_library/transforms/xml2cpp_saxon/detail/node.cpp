// galimatron
#include "galimatron/translator_library/transforms/xml2cpp_saxon/detail/node.hpp"
#include "galimatron/translator_library/transforms/layout_fingerprint.hpp"

// Saxon
#include <saxonc/XdmNode.h>


namespace galimatron::translator::transforms::xml2cpp_saxon
{

namespace
{

struct held_nodes_t
{
    XdmNode** raw = nullptr;
    int count = 0;

    held_nodes_t() = default;

    ~held_nodes_t()
    {
        if (!raw)
        {
            return;
        }
        for (int i = 0; i < count; ++i)
        {
            delete raw[i];
        }
        delete[] raw;
    }

    held_nodes_t(const held_nodes_t&) = delete;
    held_nodes_t& operator=(const held_nodes_t&) = delete;
};

std::string copy_saxon_string(const char* text)
{
    if (!text)
    {
        return {};
    }
    std::string out{ text };
    operator delete(const_cast<char*>(text));
    return out;
}

}// namespace


node_t::~node_t()
{
    if (owns_)
    {
        delete node_;
    }
}

node_t::node_t(node_t&& other) noexcept
    : node_(std::exchange(other.node_, nullptr))
    , owns_(std::exchange(other.owns_, false))
{
}

node_t& node_t::operator=(node_t&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    if (owns_)
    {
        delete node_;
    }
    node_ = std::exchange(other.node_, nullptr);
    owns_ = std::exchange(other.owns_, false);
    return *this;
}

node_t::operator bool() const
{
    return node_ != nullptr;
}

node_t node_t::adopt(XdmNode* node, const bool owns)
{
    node_t out;
    out.node_ = node;
    out.owns_ = owns && node != nullptr;
    return out;
}

std::string_view node_t::local_name() const
{
    if (!node_)
    {
        return {};
    }
    const char* const local = node_->getLocalName();
    if (!local)
    {
        return {};
    }
    return local;
}

bool node_t::is_element() const
{
    return node_ && node_->getNodeKind() == ELEMENT;
}

bool node_t::is_text() const
{
    return node_ && node_->getNodeKind() == TEXT;
}

std::string node_t::text() const
{
    if (!node_)
    {
        return {};
    }
    return copy_saxon_string(node_->getStringValue());
}

node_t node_t::parent() const
{
    if (!node_)
    {
        return {};
    }
    return adopt(node_->getParent(), true);
}

std::vector<node_t> node_t::children() const
{
    if (!node_)
    {
        return {};
    }

    held_nodes_t held;
    held.raw = node_->getChildren();
    if (!held.raw)
    {
        return {};
    }
    held.count = node_->getChildCount();

    std::vector<node_t> out;
    out.reserve(held.count);
    for (int i = 0; i < held.count; ++i)
    {
        out.push_back(adopt(held.raw[i], true));
        held.raw[i] = nullptr;
    }
    return out;
}

node_t node_t::child(const std::string_view name) const
{
    for (auto& item : children())
    {
        if (item.is_element() && item.local_name() == name)
        {
            return std::move(item);
        }
    }
    return {};
}

std::vector<node_t> node_t::children(const std::string_view name) const
{
    std::vector<node_t> matched;
    for (auto& item : children())
    {
        if (item.is_element() && item.local_name() == name)
        {
            matched.push_back(std::move(item));
        }
    }
    return matched;
}

node_t::attribute_t node_t::attribute(const std::string_view name) const
{
    attribute_t out;
    if (!is_element())
    {
        return out;
    }

    const std::string key{ name };
    const char* const value = node_->getAttributeValue(key.c_str());
    if (!value)
    {
        return out;
    }

    out.present = true;
    out.text = copy_saxon_string(value);
    return out;
}

std::vector<node_t::attribute_record_t> node_t::attributes() const
{
    if (!is_element())
    {
        return {};
    }

    held_nodes_t held;
    held.raw = node_->getAttributeNodes();
    if (!held.raw)
    {
        return {};
    }
    held.count = node_->getAttributeCount();

    std::vector<node_t> owned;
    owned.reserve(held.count);
    for (int i = 0; i < held.count; ++i)
    {
        owned.push_back(adopt(held.raw[i], true));
        held.raw[i] = nullptr;
    }

    return
        owned
        | ranges::views::transform([](const node_t& attribute)
        {
            return attribute_record_t{
                .name = std::string{ attribute.local_name() },
                .value = attribute.text()
            };
        })
        | ranges::to<std::vector<attribute_record_t>>()
        ;
}

namespace
{

struct fingerprint_access_t
{
    static std::string_view name(const node_t& node)
    {
        return node.local_name();
    }

    static std::vector<node_t::attribute_record_t> attributes(const node_t& node)
    {
        return node.attributes();
    }

    static std::string_view attribute_name(const node_t::attribute_record_t& attribute)
    {
        return attribute.name;
    }

    static std::string_view attribute_value(const node_t::attribute_record_t& attribute)
    {
        return attribute.value;
    }

    static std::vector<node_t> children(const node_t& node)
    {
        return node.children();
    }

    static bool is_element(const node_t& node)
    {
        return node.is_element();
    }

    static bool is_text(const node_t& node)
    {
        return node.is_text();
    }

    static std::string text(const node_t& node)
    {
        return node.text();
    }
};

}// namespace

std::string fingerprint(const node_t& node)
{
    return layout::detail::fingerprint<fingerprint_access_t>(node);
}

}// namespace galimatron::translator::transforms::xml2cpp_saxon
