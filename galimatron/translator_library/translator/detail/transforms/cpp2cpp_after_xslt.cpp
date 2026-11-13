// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/builtin_transform_name.hpp"
#include "galimatron/translator_library/translator/detail/xslt_seal.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"

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


transform_t translator_t::get_cpp2cpp_after_xslt_transform()
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
            .state_type = CPP
        },
        .config_transform = store_builtin_transform(builtin_transform_name::cpp2cpp_after_xslt),
        .job = [&](const transform_t&, tf::Subflow& subflow) -> file_contents_with_paths_ot
        {
            compiled_state_.reset();
            language_state_ = reassign_sealed_edits(language_before_xslt_, language_state_);
            language_before_xslt_.reset();
            // a user xslt does not rewrite a hash. the language is checked as it is after gal2cpp
            compiled_state_ = transforms::cpp2cpp_validate::validate_language(language_state_, subflow);
            return {};
        },
        .is_skipping = [&](const transform_t&) -> bool
        {
            return !language_before_xslt_ || !language_state_;
        }
    };
}

}// namespace galimatron::translator::translator
