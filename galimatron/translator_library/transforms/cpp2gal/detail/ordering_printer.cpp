// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/ordering_printer.hpp"
#include "galimatron/translator_library/transforms/cpp2gal/detail/tag_printer.hpp"


namespace galimatron::translator::transforms::cpp2gal
{

bool ordering_printer_t::fits(const int tab, const std::string_view text) const
{
    const auto indent = tab > 0 ? static_cast<std::size_t>(tab) : std::size_t{0};
    return indent + text.size() <= tag_printer_t::max_columns;
}

ordering_printer_t::printed_lines_t ordering_printer_t::print(
    const int indent,
    const ordering_statement_t& args
) const
{
    const auto header = std::format("{} {} {}",
        tags::ordering_gal::ordering,
        args.name,
        GaL::EQ
    );

    const auto flat_parts =
        args.ordering_parts
        | ranges::views::transform([](const ordering_statement_t::ordering_part_t& part)
        {
            return tag_printer_t::bracketed(GaL::LC, GaL::RC, part);
        })
        | ranges::to<strings_t>()
        ;
    const auto body = boost::algorithm::join(flat_parts, GaL::COMMA_WITH_SPACE);
    auto text = body.empty()
        ? std::format("{}{}",
            header,
            GaL::SEMI
        )
        : std::format("{} {}{}",
            header,
            body,
            GaL::SEMI
        );

    printed_lines_t lines;
    if (args.ordering_parts.empty() || fits(indent, text))
    {
        lines.push_back({
            .tab = indent,
            .content = std::move(text)
        });
        return lines;
    }

    if (args.ordering_parts.size() == 1)
    {
        const auto brace = std::format("{} {}",
            header,
            GaL::LC
        );
        if (fits(indent, brace))
        {
            lines.push_back({
                .tab = indent,
                .content = brace
            });
        }
        else
        {
            lines.push_back({
                .tab = indent,
                .content = header
            });
            lines.push_back({
                .tab = indent,
                .content = std::string(GaL::LC)
            });
        }
        render_names(args.ordering_parts.front(), indent + tab_size, lines);
        lines.push_back({
            .tab = indent,
            .content = std::format("{}{}",
                GaL::RC,
                GaL::SEMI
            )
        });
        return lines;
    }

    lines.push_back({
        .tab = indent,
        .content = header
    });
    const auto child_indent = indent + tab_size;
    for (const auto& [index, part] : ranges::views::enumerate(args.ordering_parts))
    {
        render_part(
            part,
            child_indent,
            lines,
            static_cast<std::size_t>(index) + 1 == args.ordering_parts.size()
        );
    }
    return lines;
}

void ordering_printer_t::render_part(
    const ordering_statement_t::ordering_part_t& part,
    const int indent,
    printed_lines_t& lines,
    const bool last
) const
{
    auto flat = tag_printer_t::bracketed(GaL::LC, GaL::RC, part);
    flat.append(last ? GaL::SEMI : GaL::COMMA);
    if (fits(indent, flat))
    {
        lines.push_back({
            .tab = indent,
            .content = std::move(flat)
        });
        return;
    }

    lines.push_back({
        .tab = indent,
        .content = std::string(GaL::LC)
    });
    render_names(part, indent + tab_size, lines);
    auto close = std::string(GaL::RC);
    close.append(last ? GaL::SEMI : GaL::COMMA);
    lines.push_back({
        .tab = indent,
        .content = std::move(close)
    });
}

void ordering_printer_t::render_names(
    const ordering_statement_t::ordering_part_t& part,
    const int indent,
    printed_lines_t& lines
) const
{
    if (part.empty())
    {
        return;
    }

    const auto joined = boost::algorithm::join(part, GaL::COMMA_WITH_SPACE);
    if (fits(indent, joined))
    {
        lines.push_back({
            .tab = indent,
            .content = joined
        });
        return;
    }

    auto broken =
        part
        | ranges::views::enumerate
        | ranges::views::transform([&](const auto& item)
        {
            const auto& [index, name] = item;
            auto text = std::string{ name };
            if (static_cast<std::size_t>(index) + 1 != part.size())
            {
                text.append(GaL::COMMA);
            }
            return printed_line_t{
                .tab = indent,
                .content = std::move(text)
            };
        })
        ;
    lines |= ranges::actions::push_back(broken);
}

}// namespace galimatron::translator::transforms::cpp2gal
