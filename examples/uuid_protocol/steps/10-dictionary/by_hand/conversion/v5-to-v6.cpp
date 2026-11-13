// uuid_protocol
#include "v5-to-v6.hpp"
#include "uuid_protocol/steps/10-dictionary/by_hand/state_internal.hpp"


namespace uuid_protocol::conversion
{

// copies a v5 fog into the last watch. v5 sea is calm or swell
//
// testing: no
//
void apply(server_state_t& state, const v5::payload::fog_t& from)
{
    state.watch.fog.metres = from.metres;
    state.watch.fog.sea = from.sea == v5::payload::sea_state_e::swell
        ? v6::payload::sea_state_e::swell
        : v6::payload::sea_state_e::calm;
    if (state.watch.day.empty())
    {
        state.watch.day = "unknown";
    }
}

// copies a v5 lamp into the last watch
//
// testing: no
//
void apply(server_state_t& state, const v5::payload::lamp_t& from)
{
    state.watch.lamp.lit = from.lit;
    state.watch.lamp.candela = from.candela;
    if (state.watch.day.empty())
    {
        state.watch.day = "unknown";
    }
}

// copies a v5 vessel into the last vessel. v5 has no name
//
// testing: no
//
void apply(server_state_t& state, const v5::payload::vessel_t& from)
{
    state.vessel.kind = from.kind == v5::payload::vessel_kind_e::steamer
        ? v6::payload::vessel_kind_e::steamer
        : v6::payload::vessel_kind_e::sail;
    state.vessel.bearing = from.bearing;
    state.vessel.name = {};
}

}// namespace uuid_protocol::conversion
