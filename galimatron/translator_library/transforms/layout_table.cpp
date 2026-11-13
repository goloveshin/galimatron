// galimatron
#include "galimatron/translator_library/transforms/layout_table.hpp"
#include "galimatron/translator_library/transforms/layout_fingerprint.hpp"


namespace galimatron::translator::layout
{

namespace
{

struct fingerprint_access_t
{
    static std::string_view name(const pugi::xml_node& node)
    {
        return node.name();
    }

    static auto attributes(const pugi::xml_node& node)
    {
        return node.attributes();
    }

    static std::string_view attribute_name(const pugi::xml_attribute& attribute)
    {
        return attribute.name();
    }

    static std::string_view attribute_value(const pugi::xml_attribute& attribute)
    {
        return attribute.value();
    }

    static auto children(const pugi::xml_node& node)
    {
        return node.children();
    }

    static bool is_element(const pugi::xml_node& node)
    {
        return node.type() == pugi::node_element;
    }

    static bool is_text(const pugi::xml_node& node)
    {
        return node.type() == pugi::node_cdata || node.type() == pugi::node_pcdata;
    }

    static std::string_view text(const pugi::xml_node& node)
    {
        return node.value();
    }
};

}// namespace

std::string fingerprint(const pugi::xml_node& node)
{
    return detail::fingerprint<fingerprint_access_t>(node);
}

}// namespace galimatron::translator::layout
