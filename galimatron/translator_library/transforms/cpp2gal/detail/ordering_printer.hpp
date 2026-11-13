#pragma once

// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"


namespace galimatron::translator::transforms::cpp2gal
{

// every ordering, including one that had stored source text
// one line when the statement, including its indent, fits in 100 columns
// one part breaks like a tag. several parts stay separate brace groups
//
struct ordering_printer_t
{
    int tab_size = static_cast<int>(cpp2gal_t::tab_size);

    struct printed_line_t
    {
        int tab = 0;
        std::string content;
    };
    using printed_lines_t = std::deque<printed_line_t>;

    bool fits(int, std::string_view) const;
    printed_lines_t print(int, const ordering_statement_t&) const;

private:
    void render_part(
        const ordering_statement_t::ordering_part_t&,
        int,
        printed_lines_t&,
        bool last
    ) const;
    void render_names(
        const ordering_statement_t::ordering_part_t&,
        int,
        printed_lines_t&
    ) const;
};

}// namespace galimatron::translator::transforms::cpp2gal
