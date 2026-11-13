#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"


namespace galimatron::translator::translator
{

// Taskflow writes each node id as p plus the object's address.
// this replaces those ids with n1, n2, ... in the order they appear. labels stay
//
// testing: yes (unit)
//
std::string readable_taskflow_dump(const std::string& dump);

}// namespace galimatron::translator::translator
