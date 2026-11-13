// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/tag_printer.hpp"
#include "galimatron/common_library/exception_augment.hpp"


namespace galimatron::translator::transforms::cpp2gal
{

std::deque<vertex_descriptor_t> tag_printer_t::children(const vertex_descriptor_t vd) const
{
    std::deque<vertex_descriptor_t> result;
    for (const auto vd_child : subrange(boost::adjacent_vertices(vd, graph)))
    {
        result.push_back(vd_child);
    }
    return result;
}

vertex_descriptor_t tag_printer_t::unwrap(const vertex_descriptor_t vd) const
{
    if (graph[vd].entity_type != entity_et::TAG_VALUE)
    {
        return vd;
    }

    const auto inner = children(vd);
    if (inner.size() != 1)
    {
        return vd;
    }
    return inner.front();
}

bool tag_printer_t::fits(const int tab, const std::string_view text) const
{
    const auto indent = tab > 0 ? static_cast<std::size_t>(tab) : std::size_t{0};
    return indent + text.size() <= max_columns;
}

string_delimiter_t tag_printer_t::delimiter_at(const index_t index) const
{
    if (!delimiters || index >= delimiters->size())
    {
        return {};
    }
    return (*delimiters)[index];
}

// wraps the trimmed text in its delimiter. a closer that the text now contains gains dashes
//
std::string tag_printer_t::quote_text(const string_delimiter_t& delimiter, const std::string_view text) const
{
    const auto quote_inside = boost::contains(text, std::string(1, delimiter.quote));
    if (!delimiter.raw && !quote_inside)
    {
        return std::format("{}{}{}", delimiter.quote, text, delimiter.quote);
    }

    auto dashes = delimiter.raw ? delimiter.dashes : std::string{};
    const auto closer = [&]()
    {
        return std::format("){}{}", dashes, delimiter.quote);
    };
    while (boost::contains(text, closer()))
    {
        dashes.push_back('-');
    }
    return std::format("R{}{}({}){}{}", delimiter.quote, dashes, text, dashes, delimiter.quote);
}

// quotes the text. a template string keeps the _te mark
//
std::string tag_printer_t::quoted(const tag_string_t& value) const
{
    auto text = quote_text(delimiter_at(value.delimiter), value.value);
    if (value.is_template)
    {
        text += "_te";
    }
    return text;
}

std::string tag_printer_t::quoted_key(const tag_keyvalue_t& keyvalue) const
{
    if (!keyvalue.key)
    {
        return {};
    }
    return quote_text(delimiter_at(keyvalue.key_delimiter), *keyvalue.key);
}

std::string tag_printer_t::bracketed(
    const std::string_view open,
    const std::string_view close,
    const strings_t& parts
)
{
    if (parts.empty())
    {
        return std::format("{} {}",
            open,
            close
        );
    }
    return std::format("{} {} {}",
        open,
        boost::algorithm::join(parts, GaL::COMMA_WITH_SPACE),
        close
    );
}

string_ot tag_printer_t::flat_line(const vertex_descriptor_t vd) const
{
    const auto payload = unwrap(vd);
    const auto& vertex = graph[payload];

    if (vertex.entity_type == entity_et::TAG_STRING)
    {
        return quoted(std::get<tag_string_t>(vertex.entity));
    }

    if (vertex.entity_type == entity_et::TAG_ARRAY)
    {
        strings_t parts;
        for (const auto vd_child : children(payload))
        {
            const auto element = unwrap(vd_child);
            if (graph[element].entity_type != entity_et::TAG_STRING)
            {
                return {};
            }
            parts.push_back(*flat_line(element));
        }
        return bracketed(GaL::LS, GaL::RS, parts);
    }

    if (vertex.entity_type == entity_et::TAG_RECORD)
    {
        strings_t parts;
        for (const auto vd_child : children(payload))
        {
            if (graph[vd_child].entity_type != entity_et::TAG_KEYVALUE)
            {
                return {};
            }
            const auto& keyvalue = std::get<tag_keyvalue_t>(graph[vd_child].entity);
            const auto value_nodes = children(vd_child);
            if (value_nodes.size() != 1)
            {
                return {};
            }
            const auto value = unwrap(value_nodes.front());
            if (graph[value].entity_type != entity_et::TAG_STRING)
            {
                return {};
            }
            const auto text = *flat_line(value);
            if (keyvalue.key)
            {
                parts.push_back(std::format("{}: {}",
                    quoted_key(keyvalue),
                    text
                ));
            }
            else
            {
                parts.push_back(text);
            }
        }
        return bracketed(GaL::LC, GaL::RC, parts);
    }

    return {};
}

string_ot tag_printer_t::flat_tag_body(const vertex_descriptor_t vd_values) const
{
    strings_t parts;
    for (const auto vd_child : children(vd_values))
    {
        const auto text = flat_line(vd_child);
        if (!text)
        {
            return {};
        }
        parts.push_back(*text);
    }
    return bracketed(GaL::LC, GaL::RC, parts);
}

void tag_printer_t::render_payload(
    const vertex_descriptor_t vd,
    const int indent,
    printed_lines_t& lines,
    const bool with_comma
) const
{
    const auto payload = unwrap(vd);
    const auto type = graph[payload].entity_type;

    if (type == entity_et::TAG_STRING)
    {
        auto line = quoted(std::get<tag_string_t>(graph[payload].entity));
        if (with_comma)
        {
            line.append(GaL::COMMA);
        }
        lines.push_back({
            .tab = indent,
            .content = std::move(line)
        });
        return;
    }

    if (type == entity_et::TAG_ARRAY || type == entity_et::TAG_RECORD)
    {
        render_container(payload, indent, lines, with_comma);
        return;
    }

    unexpected(type);
}

void tag_printer_t::render_keyvalue(
    const vertex_descriptor_t vd,
    const int indent,
    printed_lines_t& lines,
    const bool with_comma
) const
{
    const auto& keyvalue = std::get<tag_keyvalue_t>(graph[vd].entity);
    const auto value_nodes = children(vd);
    if (value_nodes.size() != 1)
    {
        unexpected(graph[vd].entity_type);
    }

    const auto payload = unwrap(value_nodes.front());
    const auto prefix = keyvalue.key
        ? std::format("{}: ", quoted_key(keyvalue))
        : std::string{};
    const auto type = graph[payload].entity_type;

    if (type == entity_et::TAG_STRING)
    {
        auto line = prefix + quoted(std::get<tag_string_t>(graph[payload].entity));
        if (with_comma)
        {
            line.append(GaL::COMMA);
        }
        lines.push_back({
            .tab = indent,
            .content = std::move(line)
        });
        return;
    }

    if (const auto text = flat_line(payload))
    {
        auto line = prefix + *text;
        if (with_comma)
        {
            line.append(GaL::COMMA);
        }
        if (fits(indent, line))
        {
            lines.push_back({
                .tab = indent,
                .content = std::move(line)
            });
            return;
        }
    }

    if (type != entity_et::TAG_ARRAY && type != entity_et::TAG_RECORD)
    {
        unexpected(type);
    }

    const auto open = std::string(type == entity_et::TAG_ARRAY ? GaL::LS : GaL::LC);
    const auto header = prefix + open;
    if (fits(indent, header))
    {
        lines.push_back({
            .tab = indent,
            .content = header
        });
        render_inside(payload, indent, lines, with_comma);
        return;
    }

    if (keyvalue.key)
    {
        lines.push_back({
            .tab = indent,
            .content = std::format("{}:", quoted_key(keyvalue))
        });
    }
    render_payload(payload, indent, lines, with_comma);
}

void tag_printer_t::render_container(
    const vertex_descriptor_t vd,
    const int indent,
    printed_lines_t& lines,
    const bool with_comma
) const
{
    const auto payload = unwrap(vd);
    const auto type = graph[payload].entity_type;
    if (type != entity_et::TAG_ARRAY && type != entity_et::TAG_RECORD)
    {
        unexpected(type);
    }

    if (const auto text = flat_line(payload))
    {
        auto line = *text;
        if (with_comma)
        {
            line.append(GaL::COMMA);
        }
        if (fits(indent, line))
        {
            lines.push_back({
                .tab = indent,
                .content = std::move(line)
            });
            return;
        }
    }

    lines.push_back({
        .tab = indent,
        .content = std::string(type == entity_et::TAG_ARRAY ? GaL::LS : GaL::LC)
    });
    render_inside(payload, indent, lines, with_comma);
}

void tag_printer_t::render_inside(
    const vertex_descriptor_t vd,
    const int indent,
    printed_lines_t& lines,
    const bool with_comma
) const
{
    const auto type = graph[vd].entity_type;
    const auto items = children(vd);
    const auto child_indent = indent + tab_size;
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const bool last = i + 1 == items.size();
        if (type == entity_et::TAG_ARRAY)
        {
            render_payload(items[i], child_indent, lines, !last);
        }
        else
        {
            render_keyvalue(items[i], child_indent, lines, !last);
        }
    }

    auto close_line = std::string(type == entity_et::TAG_ARRAY ? GaL::RS : GaL::RC);
    if (with_comma)
    {
        close_line.append(GaL::COMMA);
    }
    lines.push_back({
        .tab = indent,
        .content = std::move(close_line)
    });
}

void tag_printer_t::unexpected(const entity_et type) const
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::cpp2gal{},
        exception::e_what_info{ std::format("unexpected entity '{}' in cpp2gal", to_string(type)) }
    );
}

}// namespace galimatron::translator::transforms::cpp2gal
