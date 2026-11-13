
#pragma once

#include "v5/v5.hpp"


namespace uuid_protocol
{

struct server_state_t;

namespace conversion
{


void apply(server_state_t& state, const v5::payload::fog_t& from);


void apply(server_state_t& state, const v5::payload::lamp_t& from);


void apply(server_state_t& state, const v5::payload::vessel_t& from);


}// namespace conversion

}// namespace uuid_protocol