// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"

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


transform_t translator_t::get_xml2xml_transform(const galimatron_config::transform_t& config_transform)
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
            .state_type = SAXON
        },
        .config_transform = config_transform,
        .job = [&](const transform_t& tf, tf::Subflow&) -> file_contents_with_paths_ot
        {
            auto& saxon = any_transform_states_.state_slot(state_slot_et::SAXON_SESSION);
            return transforms::xml2xml::operation_t::apply(saxon, tf.config_transform);
        },
        .origin = detail::origin_et::USER
    };
}

}// namespace galimatron::translator::translator
