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


// loads that pugixml document into Saxon.
// the sequence builder inserts this only before a language xslt.
// prepare_testing_transforms inserts this when xml2cpp is on
//
// testing: no
//
transform_t translator_t::get_xml2saxon_transform()
{
    using enum transform_sequence_builder::domain_et;
    using enum transform_sequence_builder::state_type_et;

    return {
        .incoming_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = XML
        },
        .outgoing_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = SAXON
        },
        .config_transform = store_builtin_transform(builtin_transform_name::xml2saxon),
        .job = [&](const transform_t&, tf::Subflow&) -> file_contents_with_paths_ot
        {
            auto& saxon = any_transform_states_.state_slot(state_slot_et::SAXON_SESSION);
            saxon = transforms::xml2xml::prepare({ .config = args_.saxon_processor });
            transforms::xml2xml::operation_t::adopt(saxon, translate_cpp2xml(
                false,
                builtin_transform_xml::cpp2xml_description
            ));
            // testing keeps the language graph: a later cpp2gal still prints it
            if (!args_.testing)
            {
                language_state_.reset();
            }
            return {};
        }
    };
}

}// namespace galimatron::translator::translator
