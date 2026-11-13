// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/builtin_transform_name.hpp"
#include "galimatron/translator_library/translator/builtin_transform_xml.hpp"

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


transform_t translator_t::get_post_cpp2post_xml_transform()
{
    using enum transform_sequence_builder::domain_et;
    using enum transform_sequence_builder::state_type_et;

    return {
        .incoming_domain_and_state = {
            .domain = POST_LANGUAGE,
            .state_type = CPP
        },
        .outgoing_domain_and_state = {
            .domain = POST_LANGUAGE,
            .state_type = XML
        },
        .config_transform = store_builtin_transform(
            builtin_transform_name::post_cpp2post_xml,
            path_t{ builtin_transform_xml::cpp2post_xml }
        ),
        .job = [&](const transform_t&, tf::Subflow&) -> file_contents_with_paths_ot
        {
            return {};
        },
        .is_skipping = [&](const transform_t&) -> bool
        {
            return !compiled_state_ || !language_state_;
        },
        // execute prints the compiled document with pugixml
        .print_as_compiled_document = true,
        .description = builtin_transform_xml::sealed_description
    };
}

}// namespace galimatron::translator::translator
