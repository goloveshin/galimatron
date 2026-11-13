// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/builtin_transform_name.hpp"
#include "galimatron/translator_library/translator/builtin_transform_xml.hpp"

// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/transforms/xml2cpp_saxon/xml2cpp_saxon.hpp"

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


transform_t translator_t::get_xml2cpp_transform()
{
    using enum transform_sequence_builder::domain_et;
    using enum transform_sequence_builder::state_type_et;

    return {
        .incoming_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = SAXON
        },
        .outgoing_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = CPP
        },
        .config_transform = store_builtin_transform(
            builtin_transform_name::xml2cpp,
            path_t{ builtin_transform_xml::xml2cpp }
        ),
        .job = [&](const transform_t&, tf::Subflow&) -> file_contents_with_paths_ot
        {
            auto& saxon = any_transform_states_.state_slot(state_slot_et::SAXON_SESSION);
            compiled_state_.reset();
            language_state_ = transforms::xml2cpp_saxon::translate(
                saxon,
                &any_transform_states_.layout_table()
            );
            saxon.reset();
            return {};
        },
        .description = builtin_transform_xml::xml2cpp_description
    };
}

}// namespace galimatron::translator::translator
