// uuid_protocol
#include "v3-to-v6.hpp"
#include "uuid_protocol/steps/09-watch/by_hand/state_internal.hpp"


namespace uuid_protocol::conversion
{

// copies a v3 lamp into the last watch. v3 has lit and no candela
//
// testing: no
//
void apply(server_state_t& state, const v3::payload::lamp_t& from)
{
    state.watch.lamp.lit = from.lit;
    if (state.watch.day.empty())
    {
        state.watch.day = "unknown";
    }
}

}// namespace uuid_protocol::conversion
