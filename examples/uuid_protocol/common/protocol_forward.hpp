#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"


namespace uuid_protocol
{

// one group per system message of one version

constexpr group_t no_group = 0;
constexpr group_t hello_group = 1;
constexpr group_t bye_group = 2;
constexpr group_t ok_group = 3;
constexpr group_t error_group = 4;

}// namespace uuid_protocol
