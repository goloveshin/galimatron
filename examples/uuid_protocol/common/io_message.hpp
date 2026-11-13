#pragma once

// uuid_protocol
#include "uuid_protocol/common/client_state.hpp"
#include "uuid_protocol/common/state.hpp"


namespace uuid_protocol
{

// one definition, in the translation unit that defines server_io_message.
// GNU ld links the step archive before the common library and scans each
// archive once. the client link needs both functions in this object file.
//
// testing: no
//
std::string& io_message(const any_state_t& any_state)
{
    if (const auto* client = std::any_cast<std::shared_ptr<client_state_t>>(&any_state))
    {
        return (*client)->io_message;
    }
    return server_io_message(any_state);
}

}// namespace uuid_protocol
