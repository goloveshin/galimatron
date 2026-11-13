// galimatron
#include "galimatron/translator_library/state/wizard/forward.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state::wizard
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_wiz);
}


std::string to_string(const context_et& context)
{
    using enum context_et;

    switch (context)
    {
        case CHUNK:
            return "chunk";
        case ROOT_AGGREGATE:
            return "root-aggregate";
        case ROOT_AGGREGATE_VERSION_DECLARATIONS:
            return "root-aggregate-version-declarations";
        case ROOT_AGGREGATE_VERSION_DEFINITION:
            return "root-aggregate-version-definition";
        case AGGREGATE:
            return "aggregate";
        case AGGREGATE_VERSION_DECLARATIONS:
            return "aggregate-version-declarations";
        case AGGREGATE_VERSION_DEFINITION:
            return "aggregate-version-definition";
    }
}

std::string to_string(const command_et& command)
{
    using enum command_et;

    switch (command)
    {
        case CREATE_ROOT_AGGREGATE:
            return "create root aggregate";
        case CREATE_AGGREGATE:
            return "create aggregate";
        case COPY_AGGREGATE:
            return "copy aggregate";
        case CONTINUE_AGGREGATE:
            return "continue aggregate";
        case DELETE_AGGREGATE:
            return "delete aggregate";
        case CREATE_VERSION:
            return "create version";
        case DELETE_VERSION:
            return "delete version";
        case COLLAPSE_VERSION:
            return "collapse version";
        case CREATE_LABEL:
            return "create label";
        case CREATE_TAG:
            return "create tag";
        case DELETE_TAG:
            return "delete tag";
        case CREATE_ORDERING:
            return "create ordering";
        case CREATE_HASH:
            return "create hash";
        case DELETE_HASH:
            return "delete hash";
        case BATCH:
            return "batch";
    }
}

}// namespace galimatron::translator::state::wizard
