#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"


namespace uuid_protocol
{

// client session state. io_notify stores the text
//
// testing: no
//
state_operations_t make_client_state();

// server session state. a step's by_hand/server_state.cpp defines this.
// a step without that file uses the io_message-only state
//
// testing: no
//
state_operations_t make_server_state();

// the server text inside the any. the same file defines this
// throws std::bad_any_cast when any_state is not that step's server state
//
// testing: no
//
std::string& server_io_message(const any_state_t& any_state);

// the text io_notify stored on this endpoint
// throws std::bad_any_cast when any_state is neither the client state nor the server state
// the server-state translation unit defines this, beside server_io_message
//
// testing: no
//
std::string& io_message(const any_state_t& any_state);

}// namespace uuid_protocol
