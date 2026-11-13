// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr);
}


std::string to_string(const entity_et& entity)
{
    return std::string(magic_enum::enum_name(entity));
}

}// namespace galimatron::translator::state
