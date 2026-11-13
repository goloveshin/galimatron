
#pragma once

#include "v3/v3.hpp"


namespace uuid_protocol
{

struct server_state_t;

namespace conversion
{


void apply(server_state_t& state, const v3::payload::lamp_t& from);


}// namespace conversion

}// namespace uuid_protocol