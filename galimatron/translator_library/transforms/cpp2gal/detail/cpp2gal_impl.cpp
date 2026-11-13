// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2gal
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2g);
}

namespace
{

// the printed lines of one comment. a trailing newline does not add an empty line
//
strings_t lines_of_comment(std::string text)
{
    boost::algorithm::replace_all(text, GaL::CRLF, GaL::NEWLINE);
    boost::algorithm::replace_all(text, GaL::CR, GaL::NEWLINE);
    if (!text.empty() && text.back() == GaL::NEWLINE_CHAR)
    {
        text.pop_back();
    }

    strings_t lines;
    boost::split(lines, text, boost::is_any_of(GaL::NEWLINE));
    return
        lines
        | ranges::views::filter([](const std::string& line){ return !line.empty(); })
        | ranges::to<strings_t>()
        ;
}

}// namespace


cpp2gal_t::cpp2gal_t(const translate_t& args)
    : args_(args)
    , graph_(args_.language_state->graph())
{
}

file_contents_with_paths_t cpp2gal_t::translate()
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::cpp2gal{}
    );

    // root and the walk of its children — 2 levels until the chunk text
    tab_count_.push_back(-static_cast<int>(tab_size) * 2);
    walk_children({
        .vd = args_.language_state->root_node()
    });
    return contents_;
}

vertex_descriptor_ot cpp2gal_t::walk_children(const walk_children_t& args)
{
    vertex_descriptor_ot skipped;

    if (args.tabulate_on_enter)
    {
        tab_count_.push_back(tab_count_.back() + static_cast<int>(tab_size));
    }
    {
        for (const auto vd_child : subrange(boost::adjacent_vertices(args.vd, graph_)))
        {
            if (args.skip && graph_[vd_child].entity_type == *args.skip)
            {
                skipped = vd_child;
            }
            else
            {
                emit(vd_child);
            }
        }
    }
    if (args.tabulate_on_exit)
    {
        if (!tab_count_.empty())
        {
            tab_count_.pop_back();
        }
    }

    return skipped;
}

void cpp2gal_t::emit(const vertex_descriptor_t vd)
{
    const auto& vertex = graph_[vd];

    std::visit(overloads_t{
        [&](const chunk_t& args)
        {
            emit_chunk(vd, args);
        },
        [&](const root_aggregate_t& args)
        {
            emit_root_aggregate(vd, args);
        },
        [&](const root_aggregate_version_declarations_t& args)
        {
            emit_root_aggregate_version_declarations(vd, args);
        },
        [&](const root_aggregate_version_declaration_t& args)
        {
            emit_root_aggregate_version_declaration(vd, args);
        },
        [&](const root_aggregate_version_definitions_t& args)
        {
            emit_root_aggregate_version_definitions(vd, args);
        },
        [&](const root_aggregate_version_definition_t& args)
        {
            emit_root_aggregate_version_definition(vd, args);
        },
        [&](const aggregate_t& args)
        {
            emit_aggregate(vd, args);
        },
        [&](const aggregate_version_declarations_t& args)
        {
            emit_aggregate_version_declarations(vd, args);
        },
        [&](const aggregate_version_declaration_t& args)
        {
            emit_aggregate_version_declaration(vd, args);
        },
        [&](const aggregate_version_definitions_t& args)
        {
            emit_aggregate_version_definitions(vd, args);
        },
        [&](const aggregate_version_definition_t& args)
        {
            emit_aggregate_version_definition(vd, args);
        },
        [&](const tag_statement_t& args)
        {
            emit_tag_statement(vd, args);
        },
        [&](const tag_value_t& args)
        {
            emit_tag_value(vd, args);
        },
        [&](const tag_record_t& args)
        {
            emit_tag_record(vd, args);
        },
        [&](const tag_keyvalue_t& args)
        {
            emit_tag_keyvalue(vd, args);
        },
        [&](const tag_array_t& args)
        {
            emit_tag_array(vd, args);
        },
        [&](const tag_string_t& args)
        {
            emit_tag_string(args);
        },
        [&](const label_statement_t& args)
        {
            emit_label_statement(vd, args);
        },
        [&](const ordering_statement_t& args)
        {
            emit_ordering_statement(vd, args);
        },
        [&](const operation_statement_t& args)
        {
            emit_operation_statement(vd, args);
        },
        [&](const wizard_statement_t& args)
        {
            emit_wizard_statement(vd, args);
        },
        [&](const auto&)
        {
            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::cpp2gal{},
                exception::e_what_info{ std::format("unexpected entity '{}' in cpp2gal", to_string(vertex.entity_type)) }
            );
        }
    }, vertex.entity);
}

vertex_descriptor_ot cpp2gal_t::find_child(const vertex_descriptor_t vd, const entity_et entity_type) const
{
    for (const auto vd_child : subrange(boost::adjacent_vertices(vd, graph_)))
    {
        if (graph_[vd_child].entity_type == entity_type)
        {
            return vd_child;
        }
    }
    return {};
}

void cpp2gal_t::emit_wizard_statement(const vertex_descriptor_t vd, const wizard_statement_t&)
{
    append_user_comments(vd);
}

void cpp2gal_t::append_statement_comment(const vertex_descriptor_t vd)
{
    const auto& formatting = graph_[vd].user_formatting;
    if (!formatting)
    {
        return;
    }

    const auto semi = boost::algorithm::find_last(*formatting, GaL::SEMI);
    if (semi.empty())
    {
        return;
    }

    const std::string rest(semi.end(), formatting->end());
    auto trimmed = rest;
    boost::algorithm::trim_left(trimmed);
    if (boost::starts_with(trimmed, GaL::CPP_COMMENT) || boost::starts_with(trimmed, GaL::C_COMMENT))
    {
        append_back(rest);
    }
}

bool cpp2gal_t::append_user_formatting(const vertex_descriptor_t vd)
{
    const auto& formatting = graph_[vd].user_formatting;
    if (!formatting || formatting->empty())
    {
        return false;
    }

    append_line(*formatting);
    return true;
}

void cpp2gal_t::append_user_comments(const vertex_descriptor_t vd)
{
    const auto& comments = graph_[vd].user_comments;
    const auto parts =
        comments
        | ranges::views::transform([](const std::string& comment)
        {
            return lines_of_comment(comment);
        })
        | ranges::to_vector
        ;

    for (const auto& line : parts | ranges::views::join)
    {
        append_line(line);
    }
}

void cpp2gal_t::append_empty_line()
{
    if (!lines_.empty())
    {
        boost::algorithm::trim_right_if(
            lines_.back().content,
            boost::is_any_of(GaL::NEWLINE)
        );
        lines_.push_back({
            .tab_count = tab_count_.back(),
            .content = std::string(GaL::NEWLINE)
        });
        lines_.push_back({
            .tab_count = tab_count_.back(),
            .content = std::string(GaL::NEWLINE)
        });
    }
}

void cpp2gal_t::append_line(const std::string& str)
{
    if (!lines_.empty()
        && !lines_.back().content.empty()
        && (lines_.back().content.back() != GaL::NEWLINE_CHAR))
    {
        lines_.push_back({
            .tab_count = 0,
            .content = std::string(GaL::NEWLINE)
        });
    }

    lines_.push_back({
        .tab_count = tab_count_.back(),
        .content = str
    });
}

void cpp2gal_t::append_line(const std::string_view& str)
{
    append_line(std::string(str));
}

void cpp2gal_t::append_line(const char* str)
{
    append_line(std::string(str));
}

void cpp2gal_t::append_back(const std::string_view& str)
{
    if (!lines_.empty())
    {
        lines_.back().content += str;
    }
}

std::size_t cpp2gal_t::size_of_lines(const lines_t& lines)
{
    return ranges::accumulate(lines, 0, std::plus{}, [](const auto& line)
    {
        return line.size();
    });
}

void cpp2gal_t::join_lines(const lines_t& lines, content_t& content)
{
    for (const auto& line : lines)
    {
        if (line.tab_count > 0)
            content += std::string(static_cast<std::size_t>(line.tab_count), GaL::SPACE_CHAR);
        content += line.content;
    }
}

void cpp2gal_t::trim_comma_back()
{
    if (!lines_.empty())
    {
        boost::algorithm::trim_right_if(
            lines_.back().content,
            boost::is_any_of(GaL::COMMA)
        );
    }
}

}// namespace galimatron::translator::transforms::cpp2gal
