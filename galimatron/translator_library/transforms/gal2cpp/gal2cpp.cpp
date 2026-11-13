// galimatron
#include "galimatron/translator_library/transforms/gal2cpp/gal2cpp.hpp"
#include "galimatron/translator_library/transforms/gal2cpp/detail/listener.hpp"
#include "galimatron/translator_library/parser/parser.hpp"
#include "galimatron/translator_library/parser/state.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::gal2cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_g2s);
}


holder::language_part_sptr_t translate(const translate_t& args)
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::gal2cpp{}
    );

    // we do not use global_parse_state_at for fill_state_from_content(),
    // because this is more convenient here

    auto state = std::make_shared<holder::language_part_t>();
    auto builder = builder::builder_t(*state);
    index_t aggregate_index = 0;

    parser::fill_state_from_content({
        .view_deferred_file_content_with_path = args.gal_state,
        .get_global_state = []{ return parser::global_parse_state_at{}; },
        .parse_content = [gal_version = args.gal_version](const file_content_t& content)
        {
            return parser::parse(gal_version, content);
        },
        .get_listener = [&](
            parser::global_parse_state_at& /* any_global_state */,
            const path_t& content_path
        )
        {
            using listener_t = gal2cpp::listener_t;
            return std::make_shared<listener_t>(listener_t::config_t{
                .builder = builder,
                .content_path = content_path,
                .aggregate_index = aggregate_index
            });
        },
        .walk = [](parser::single_parse_state_at& parse_state, antlr_listener_t* listener)
        {
            auto& state = std::any_cast<std::shared_ptr<parser::state_t>&>(parse_state);
            static_cast<listener_t*>(listener)->bind_token_stream(&state->token_stream());
            parser::walk(parse_state, listener);
        }
    });

    state->drop_detached_vertices();
    return state;
}

}// namespace galimatron::translator::transforms::gal2cpp
