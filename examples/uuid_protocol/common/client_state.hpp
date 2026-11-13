#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"


namespace uuid_protocol
{

// client session state. io_notify stores the text
//
// testing: no
//
struct client_state_t
{
    std::string io_message;
};

// the client state inside the any
// throws std::bad_any_cast when any_state is not the client state
//
// testing: no
//
inline client_state_t& cast_to_client_state(const any_state_t& any_state)
{
    return *std::any_cast<std::shared_ptr<client_state_t>>(any_state);
}

}// namespace uuid_protocol
