// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


void translate(const config_t& args)
{
    translator_t(args).translate();
}

}// namespace galimatron::translator::translator
