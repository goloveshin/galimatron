// uuid_protocol
#include "uuid_protocol/common/client_state.hpp"
#include "uuid_protocol/common/state.hpp"


namespace uuid_protocol
{

// testing: no
//
state_operations_t make_client_state()
{
    return {
        .any_state = std::make_shared<client_state_t>(),
        .io_notify = [](const any_state_t& any_state, const std::string& msg)
        {
            cast_to_client_state(any_state).io_message = msg;
        }
    };
}

}// namespace uuid_protocol
