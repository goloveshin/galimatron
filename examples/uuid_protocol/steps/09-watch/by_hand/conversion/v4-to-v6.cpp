// uuid_protocol
#include "v4-to-v6.hpp"
#include "uuid_protocol/steps/09-watch/by_hand/state_internal.hpp"


namespace uuid_protocol::conversion
{

// copies a v4 fog into the last watch. v4 fog is metres only
//
// testing: no
//
void apply(server_state_t& state, const v4::payload::fog_t& from)
{
    state.watch.fog.metres = from.metres;
    if (state.watch.day.empty())
    {
        state.watch.day = "unknown";
    }
}

// copies a v4 lamp into the last watch
//
// testing: no
//
void apply(server_state_t& state, const v4::payload::lamp_t& from)
{
    state.watch.lamp.lit = from.lit;
    state.watch.lamp.candela = from.candela;
    if (state.watch.day.empty())
    {
        state.watch.day = "unknown";
    }
}

}// namespace uuid_protocol::conversion
