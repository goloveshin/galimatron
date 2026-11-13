#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"
#include "v6/v6.hpp"


namespace uuid_protocol
{

namespace last = v6::payload;

// the only version this server keeps
//
// testing: no
//
struct server_state_t
{
    std::string io_message;
    last::watch_t watch;
    last::vessel_t vessel;

    // stores a watch of this version
    //
    // testing: no
    //
    void incoming(const last::watch_t& from)
    {
        watch = from;
    }

    // stores a vessel of this version
    //
    // testing: no
    //
    void incoming(const last::vessel_t& from)
    {
        vessel = from;
    }
};

// the server state inside the any
// throws std::bad_any_cast when any_state is not this server state
//
// testing: no
//
server_state_t& cast_to_server_state(const any_state_t& any_state);

}// namespace uuid_protocol
