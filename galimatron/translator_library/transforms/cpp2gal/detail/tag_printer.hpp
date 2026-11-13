#pragma once

// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"


namespace galimatron::translator::transforms::cpp2gal
{

// a tag with no stored source text
// a flat value is a string, an array of strings, or a record of strings
// one line when every value is flat and the line, including its indent, fits in 100 columns
//
struct tag_printer_t
{
    const holder::language_part_t::graph_t& graph;
    const std::vector<string_delimiter_t>* delimiters = nullptr;
    int tab_size = static_cast<int>(cpp2gal_t::tab_size);

    static constexpr std::size_t max_columns = 100;

    struct printed_line_t
    {
        int tab = 0;
        std::string content;
    };
    using printed_lines_t = std::deque<printed_line_t>;

    std::deque<vertex_descriptor_t> children(vertex_descriptor_t) const;
    vertex_descriptor_t unwrap(vertex_descriptor_t) const;
    bool fits(int, std::string_view) const;
    string_delimiter_t delimiter_at(index_t) const;
    std::string quote_text(const string_delimiter_t&, std::string_view) const;
    std::string quoted(const tag_string_t&) const;
    std::string quoted_key(const tag_keyvalue_t&) const;
    static std::string bracketed(std::string_view, std::string_view, const strings_t&);
    string_ot flat_line(vertex_descriptor_t) const;
    string_ot flat_tag_body(vertex_descriptor_t) const;
    void render_payload(vertex_descriptor_t, int, printed_lines_t&, bool) const;
    void render_keyvalue(vertex_descriptor_t, int, printed_lines_t&, bool) const;
    void render_container(vertex_descriptor_t, int, printed_lines_t&, bool) const;
    void render_inside(vertex_descriptor_t, int, printed_lines_t&, bool) const;
    [[noreturn]] void unexpected(entity_et) const;
};

}// namespace galimatron::translator::transforms::cpp2gal
