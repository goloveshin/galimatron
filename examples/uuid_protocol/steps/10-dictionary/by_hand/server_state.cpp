// uuid_protocol
#include "uuid_protocol/common/io_message.hpp"
#include "uuid_protocol/common/state.hpp"
#include "uuid_protocol/steps/10-dictionary/by_hand/state_internal.hpp"


namespace uuid_protocol
{

// testing: no
//
server_state_t& cast_to_server_state(const any_state_t& any_state)
{
    return *std::any_cast<std::shared_ptr<server_state_t>>(any_state);
}

// testing: no
//
std::string& server_io_message(const any_state_t& any_state)
{
    return cast_to_server_state(any_state).io_message;
}

// testing: no
//
state_operations_t make_server_state()
{
    return {
        .any_state = std::make_shared<server_state_t>(),
        .io_notify = [](const any_state_t& any_state, const std::string& msg)
        {
            cast_to_server_state(any_state).io_message = msg;
        }
    };
}

}// namespace uuid_protocol
