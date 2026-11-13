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


namespace tag = tags::label;
namespace tag_gal = tags::label_gal;

void cpp2gal_t::emit_label_statement(const vertex_descriptor_t vd, const label_statement_t& args)
{
    const auto static_part = [&] -> std::string
    {
        const auto vd_static = find_child(vd, entity_et::LABEL_STATIC);
        if (!vd_static)
        {
            return {};
        }

        const auto& label_static = std::get<label_static_t>(graph_[*vd_static].entity);

        const auto static_values = [&] -> std::string
        {
            return label_static.values.empty()
                ? std::string("{}")
                : std::format("{{ {} }}",
                    boost::algorithm::join(label_static.values, GaL::COMMA_WITH_SPACE)
                );
        }();

        strings_t parts;
        parts.push_back(std::format("{} {}",
            tag_gal::static_highlight,
            static_values
        ));

        if (label_static.tag_name_from)
        {
            parts.push_back(std::format("{} {{ {} }}",
                tag_gal::tag_name_from,
                *label_static.tag_name_from
            ));
        }

        return boost::algorithm::join(parts, GaL::COMMA_WITH_SPACE);
    }();

    const auto dynamic_part = [&] -> std::string
    {
        const auto vd_dynamic = find_child(vd, entity_et::LABEL_DYNAMIC);
        if (!vd_dynamic)
        {
            return {};
        }

        const auto& label_dynamic = std::get<label_dynamic_t>(graph_[*vd_dynamic].entity);
        return std::format("{} {{ {} }}",
            tag::dynamic_highlight,
            label_dynamic.value
        );
    }();

    const auto value = static_part.empty() ? dynamic_part : static_part;

    append_user_comments(vd);
    if (append_user_formatting(vd))
    {
        return;
    }

    append_line(std::format("{} {} {} {}",
        tag_gal::label,
        args.name,
        GaL::EQ,
        value
    ));

    append_back(GaL::SEMI);
}

}// namespace galimatron::translator::transforms::cpp2gal
