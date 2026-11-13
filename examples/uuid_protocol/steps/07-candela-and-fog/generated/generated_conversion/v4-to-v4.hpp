
#pragma once

#include "v4/v4.hpp"


namespace uuid_protocol
{

struct server_state_t;

namespace conversion
{


void apply(server_state_t& state, const v4::payload::lamp_t& from);


void apply(server_state_t& state, const v4::payload::fog_t& from);


}// namespace conversion

}// namespace uuid_protocol