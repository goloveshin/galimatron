// galimatron
#include "galimatron/translator_library/parser/fill_state_from_content.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::parser
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_par);
}


global_parse_state_at fill_state_from_content(const fill_state_from_content_t& args)
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::parser{}
    );

    auto global_state = args.get_global_state();

    for (const auto& file : args.view_deferred_file_content_with_path)
    {
        const auto path_info = exception::augment_path(file.path);

        auto parse_state = args.parse_content(file.deferred_content());
        auto listener = args.get_listener(global_state, file.path);
        args.walk(parse_state, listener.get());
    }

    return global_state;
}

}// namespace galimatron::translator::parser
