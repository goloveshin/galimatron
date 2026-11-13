// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"

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


namespace tag_gal = tags::aggregate_gal;

void cpp2gal_t::emit_aggregate(const vertex_descriptor_t vd, const aggregate_t& args)
{
    append_empty_line();
    append_user_comments(vd);

    append_line(std::format("{} {}",
        tag_gal::aggregate,
        args.name
    ));

    append_line(GaL::LC);
    const auto definitions = walk_children({
        .vd = vd,
        .skip = entity_et::AGGREGATE_VERSION_DEFINITIONS
    });
    append_line(GaL::RC);

    // Version definitions come after the aggregate's closing brace.
    if (definitions)
    {
        emit(*definitions);
    }
}

void cpp2gal_t::emit_aggregate_version_declarations(const vertex_descriptor_t vd, const aggregate_version_declarations_t&)
{
    append_user_comments(vd);
    append_line(tag_gal::versions);

    append_line(GaL::LC);
    {
        walk_children({ .vd = vd });
    }
    append_line(GaL::RC);
}

void cpp2gal_t::emit_aggregate_version_declaration(const vertex_descriptor_t vd, const aggregate_version_declaration_t& args)
{
    append_user_comments(vd);
    if (append_user_formatting(vd))
    {
        return;
    }

    strings_t header_parts;
    if (args.comment)
    {
        header_parts.push_back(std::format("{} = '{}'",
            tag_c::comment,
            *args.comment
        ));
    }
    if (args.hash)
    {
        header_parts.push_back(std::format("{} = '{}'",
            tag_c::hash,
            *args.hash
        ));
    }

    const auto line = [&] -> std::string
    {
        const auto index = std::to_string(args.index);

        if (args.status)
        {
            if (header_parts.empty())
            {
                return std::format("{} = {}",
                    index,
                    to_string(*args.status)
                );
            }
            return std::format("{} = {} {{ {} }}",
                index,
                to_string(*args.status),
                boost::algorithm::join(header_parts, GaL::COMMA_WITH_SPACE)
            );
        }

        if (header_parts.empty())
        {
            return index;
        }
        return std::format("{} = {{ {} }}",
            index,
            boost::algorithm::join(header_parts, GaL::COMMA_WITH_SPACE)
        );
    }();

    append_line(line);
    append_back(GaL::SEMI);
}

void cpp2gal_t::emit_aggregate_version_definitions(const vertex_descriptor_t vd, const aggregate_version_definitions_t&)
{
    walk_children({
        .vd = vd,
        .tabulate_on_enter = false,
        .tabulate_on_exit = false
    });
}

void cpp2gal_t::emit_aggregate_version_definition(const vertex_descriptor_t vd, const aggregate_version_definition_t& args)
{
    append_empty_line();
    append_user_comments(vd);

    strings_t words;
    words.push_back(std::string{ tag_gal::version });
    if (args.mode == tag_mode_et::ONCE)
    {
        words.push_back(std::string{ tags::tag_gal::once });
    }
    else if (args.mode == tag_mode_et::LOCAL)
    {
        words.push_back(std::string{ tags::tag_gal::local });
    }
    else if (args.mode == tag_mode_et::OWN)
    {
        words.push_back(std::string{ tags::tag_gal::own });
    }
    if (args.index)
    {
        words.push_back(std::to_string(*args.index));
    }
    else if (args.owner)
    {
        words.push_back(std::string{ tag_gal::before });
        words.push_back(std::to_string(*args.owner));
    }
    append_line(boost::algorithm::join(words, std::string{ GaL::SPACE }));

    append_line(GaL::LC);
    {
        walk_children({ .vd = vd });
    }
    append_line(GaL::RC);
}

}// namespace galimatron::translator::transforms::cpp2gal
