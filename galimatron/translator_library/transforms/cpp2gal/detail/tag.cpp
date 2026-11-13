// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2gal/detail/tag_printer.hpp"

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

void cpp2gal_t::emit_tag_statement(const vertex_descriptor_t vd, const tag_statement_t& args)
{
    append_user_comments(vd);
    if (append_user_formatting(vd))
    {
        return;
    }

    const tag_printer_t printer{
        .graph = graph_,
        .delimiters = &args_.language_state->string_delimiters()
    };
    const auto indent = tab_count_.back();

    strings_t header_parts;
    if (args.name)
    {
        header_parts.push_back(*args.name);
        header_parts.push_back(std::string(GaL::EQ));
    }
    if (args.mode == tag_mode_et::ONCE)
    {
        header_parts.push_back(std::string(tags::tag_gal::once));
    }
    else if (args.mode == tag_mode_et::LOCAL)
    {
        header_parts.push_back(std::string(tags::tag_gal::local));
    }
    for (const auto& label : args.labels)
    {
        header_parts.push_back(label);
    }
    const auto header = boost::algorithm::join(header_parts, GaL::SPACE);

    const auto vd_values = find_child(vd, entity_et::TAG_VALUES);
    const auto body = vd_values ? printer.flat_tag_body(*vd_values) : string_ot{};

    tag_printer_t::printed_lines_t printed;
    if (body)
    {
        auto text = header.empty()
            ? *body
            : std::format("{} {}",
                header,
                *body
            );
        text.append(GaL::SEMI);
        if (printer.fits(indent, text))
        {
            printed.push_back({
                .tab = indent,
                .content = std::move(text)
            });
        }
    }

    if (printed.empty())
    {
        const auto brace = header.empty()
            ? std::string(GaL::LC)
            : std::format("{} {}",
                header,
                GaL::LC
            );
        if (printer.fits(indent, brace))
        {
            printed.push_back({
                .tab = indent,
                .content = brace
            });
        }
        else
        {
            if (!header.empty())
            {
                printed.push_back({
                    .tab = indent,
                    .content = header
                });
            }
            printed.push_back({
                .tab = indent,
                .content = std::string(GaL::LC)
            });
        }

        if (vd_values)
        {
            const auto items = printer.children(*vd_values);
            const auto child_indent = indent + static_cast<int>(tab_size);
            for (std::size_t i = 0; i < items.size(); ++i)
            {
                printer.render_payload(
                    items[i],
                    child_indent,
                    printed,
                    i + 1 != items.size()
                );
            }
        }
        printed.push_back({
            .tab = indent,
            .content = std::format("{}{}",
                GaL::RC,
                GaL::SEMI
            )
        });
    }

    for (const auto& line : printed)
    {
        tab_count_.push_back(line.tab);
        append_line(line.content);
        tab_count_.pop_back();
    }
}

void cpp2gal_t::emit_tag_value(const vertex_descriptor_t vd, const tag_value_t&)
{
    walk_children({ .vd = vd });
    append_back(GaL::COMMA);
}

void cpp2gal_t::emit_tag_record(const vertex_descriptor_t vd, const tag_record_t&)
{
    append_line(GaL::LC);
    {
        walk_children({ .vd = vd });
        trim_comma_back();
    }
    append_line(GaL::RC);
}

void cpp2gal_t::emit_tag_keyvalue(const vertex_descriptor_t vd, const tag_keyvalue_t& args)
{
    const tag_printer_t printer{
        .graph = graph_,
        .delimiters = &args_.language_state->string_delimiters()
    };
    const auto key = args.key ? printer.quoted_key(args) : std::string{ "\"\"" };
    append_line(std::format("{} {} ",
        key,
        GaL::COL
    ));
    {
        walk_children({ .vd = vd });
    }
}

void cpp2gal_t::emit_tag_array(const vertex_descriptor_t vd, const tag_array_t&)
{
    append_line(GaL::LS);
    {
        walk_children({ .vd = vd });
        trim_comma_back();
    }
    append_line(GaL::RS);
}

void cpp2gal_t::emit_tag_string(const tag_string_t& args)
{
    const tag_printer_t printer{
        .graph = graph_,
        .delimiters = &args_.language_state->string_delimiters()
    };
    append_line(printer.quoted(args));
}

}// namespace galimatron::translator::transforms::cpp2gal
