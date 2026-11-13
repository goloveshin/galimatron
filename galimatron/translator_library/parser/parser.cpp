// galimatron
#include "galimatron/translator_library/parser/parser.hpp"
#include "galimatron/translator_library/parser/gal_version.hpp"
#include "galimatron/translator_library/parser/state.hpp"

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


single_parse_state_at parse(const file_content_t& content)
{
    auto state = std::make_shared<state_t>(*content);
    state->do_production_parse();
    return state;
}

single_parse_state_at parse(const index_t gal_version, const file_content_t& content)
{
    gal_version::require_supported(gal_version);
    return parse(content);
}

void walk(single_parse_state_at& parse_state, antlr_listener_t* listener)
{
    auto& state = std::any_cast<std::shared_ptr<state_t>&>(parse_state);
    galimatron::translator::antlr_walk(listener, state->tree());
}

}// namespace galimatron::translator::parser
