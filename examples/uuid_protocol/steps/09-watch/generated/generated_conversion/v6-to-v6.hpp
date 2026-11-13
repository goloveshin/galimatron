
#pragma once

#include "v6/v6.hpp"


namespace uuid_protocol
{

struct server_state_t;

namespace conversion
{


void apply(server_state_t& state, const v6::payload::vessel_t& from);


void apply(server_state_t& state, const v6::payload::watch_t& from);


}// namespace conversion

}// namespace uuid_protocol