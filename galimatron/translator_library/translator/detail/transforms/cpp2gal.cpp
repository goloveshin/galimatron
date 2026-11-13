// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/builtin_transform_name.hpp"
#include "galimatron/translator_library/translator/builtin_transform_xml.hpp"

// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/cpp2gal.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


transform_t translator_t::get_cpp2gal_transform()
{
    using enum transform_sequence_builder::domain_et;
    using enum transform_sequence_builder::state_type_et;

    return {
        .incoming_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = CPP
        },
        .outgoing_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = GaL
        },
        .config_transform = store_builtin_transform(
            builtin_transform_name::cpp2gal,
            path_t{ builtin_transform_xml::cpp2gal }
        ),
        .named = { detail::named_et::CPP2GAL },
        .job = [&](const transform_t&, tf::Subflow&) -> file_contents_with_paths_ot
        {
            // the slice asked for the default layout, not the stored source text
            if (args_.testing && args_.testing->drop_tag_user_formatting && language_state_)
            {
                auto& graph = language_state_->graph();
                const auto vertices = boost::vertices(graph);
                for (auto it = vertices.first; it != vertices.second; ++it)
                {
                    const auto type = graph[*it].entity_type;
                    if (type == state::entity_et::TAG_STATEMENT
                        || type == state::entity_et::ORDERING_STATEMENT)
                    {
                        graph[*it].user_formatting.reset();
                    }
                }
            }

            return transforms::cpp2gal::translate({
                .language_state = language_state_
            });
        },
        .description = builtin_transform_xml::sealed_description
    };
}

}// namespace galimatron::translator::translator
