#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::translator::state::wizard
{

enum class context_et
{
    CHUNK,

    ROOT_AGGREGATE,
    ROOT_AGGREGATE_VERSION_DECLARATIONS,
    ROOT_AGGREGATE_VERSION_DEFINITION,

    AGGREGATE,
    AGGREGATE_VERSION_DECLARATIONS,
    AGGREGATE_VERSION_DEFINITION,
};

std::string to_string(const context_et&);

}// namespace galimatron::translator::state::wizard
