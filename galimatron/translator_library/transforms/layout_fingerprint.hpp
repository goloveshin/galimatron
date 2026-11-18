#pragma once

// galimatron
#include "galimatron/translator_library/xml_tags.hpp"


namespace galimatron::translator::layout::detail
{

// one node's names, attributes, and text, then its children, without the layout mark.
// on_element is called for each element once its fingerprint is complete, children first:
// with the value of its layout mark, empty when it has none,
// and with its fingerprint. that text is valid until out changes.
// a function, because Clang 18 crashes while instantiating this as a recursive lambda
//
template <typename Access, typename Node, typename OnElement>
void append_fingerprint(std::string& out, const Node& current, OnElement& on_element)
{
    const auto begin = out.size();
    // whitespace the serializer inserts between nodes is ignored
    auto is_blank = [](const std::string_view text)
    {
        return std::ranges::all_of(text, [](const unsigned char c)
        {
            return std::isspace(c) != 0;
        });
    };

    out += Access::name(current);
    out += '\n';

    auto attributes = Access::attributes(current);
    const auto kept =
        attributes
        | ranges::views::filter([](const auto& attribute)
        {
            return Access::attribute_name(attribute) != tag_c::layout_mark;
        })
        | ranges::to_vector
        | ranges::actions::sort(ranges::less{}, [](const auto& attribute)
        {
            return std::pair{
                Access::attribute_name(attribute),
                Access::attribute_value(attribute)
            };
        })
        ;

    out +=
        kept
        | ranges::views::transform([](const auto& attribute)
        {
            return std::format(
                "{}={}\n",
                Access::attribute_name(attribute),
                Access::attribute_value(attribute)
            );
        })
        | ranges::views::join
        | ranges::to<std::string>()
        ;

    for (const auto& child : Access::children(current))
    {
        if (Access::is_element(child))
            append_fingerprint<Access>(out, child, on_element);
        else if (Access::is_text(child))
        {
            const auto text = Access::text(child);
            if (!is_blank(text))
            {
                out += text;
                out += '\n';
            }
        }
    }

    std::string_view mark;
    for (const auto& attribute : attributes)
    {
        if (Access::attribute_name(attribute) == tag_c::layout_mark)
        {
            mark = Access::attribute_value(attribute);
        }
    }
    on_element(mark, std::string_view{ out }.substr(begin));
}

// the same fingerprint when nobody asks for the elements
//
template <typename Access, typename Node>
void append_fingerprint(std::string& out, const Node& current)
{
    auto ignore = [](const std::string_view, const std::string_view){};
    append_fingerprint<Access>(out, current, ignore);
}

// the fingerprint of every element under the node, and of the node itself, in one walk.
// on_element gets the layout mark and the fingerprint, as above
//
template <typename Access, typename Node, typename OnElement>
void for_each_fingerprint(const Node& node, OnElement on_element)
{
    std::string out;
    append_fingerprint<Access>(out, node, on_element);
}

// the node's names, attributes, and text, without the layout mark
//
template <typename Access, typename Node>
std::string fingerprint(const Node& node)
{
    std::string out;
    append_fingerprint<Access>(out, node);
    return out;
}

}// namespace galimatron::translator::layout::detail
