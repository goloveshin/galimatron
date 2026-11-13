// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/builtin_transform_name.hpp"
#include "galimatron/translator_library/translator/builtin_transform_xml.hpp"

// galimatron
#include "galimatron/translator_library/transforms/gal2cpp/gal2cpp.hpp"

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


// parses GaL into the language graph.
// writes_parsed_xml stores lang-parsed.xml from this step.
// production passes false: validation writes that file after the compile
//
transform_t translator_t::get_gal2cpp_transform(
    const bool writes_parsed_xml
)
{
    using enum transform_sequence_builder::domain_et;
    using enum transform_sequence_builder::state_type_et;

    return {
        .incoming_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = GaL
        },
        .outgoing_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = CPP
        },
        .config_transform = store_builtin_transform(
            builtin_transform_name::gal2cpp,
            writes_parsed_xml ? path_ot{ path_t{ builtin_transform_xml::gal2cpp } } : path_ot{}
        ),
        .named = { detail::named_et::GAL2CPP },
        .job = [&](const transform_t&, tf::Subflow&) -> file_contents_with_paths_ot
        {
            compiled_state_.reset();
            language_state_ = galimatron::translator::transforms::gal2cpp::translate({
                .gal_state = args_.gal_state,
                .gal_version = args_.gal_version
            });
            return {};
        },
        .is_skipping = [&](const transform_t&) -> bool
        {
            // lets us simplify sequence_builder
            return static_cast<bool>(language_state_);
        },
        .description = writes_parsed_xml ? builtin_transform_xml::gal2cpp_description : std::string_view{}
    };
}

}// namespace galimatron::translator::translator
