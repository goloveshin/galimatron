#pragma once

// galimatron
#include "galimatron/translator_library/xml_tags.hpp"


namespace galimatron::translator::layout::detail
{

// the node's names, attributes, and text, without the layout mark.
// whitespace the serializer inserts between nodes is ignored
//
template <typename Access, typename Node>
std::string fingerprint(const Node& node)
{
    std::string out;

    auto is_blank = [](const std::string_view text)
    {
        return std::ranges::all_of(text, [](const unsigned char c)
        {
            return std::isspace(c) != 0;
        });
    };

    auto walk = [&](auto& self, const Node& current) -> void
    {
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
            {
                self(self, child);
            }
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
    };

    walk(walk, node);
    return out;
}

}// namespace galimatron::translator::layout::detail
