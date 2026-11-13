#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"


namespace uuid_protocol
{

// each step defines this. the shared endpoint calls it
//
// testing: no
//
void registrate();

// each step defines this. a step with no payload leaves it empty
//
// testing: no
//
void registrate_test_cases();

}// namespace uuid_protocol
