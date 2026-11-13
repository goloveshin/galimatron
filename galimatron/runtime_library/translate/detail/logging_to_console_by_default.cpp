// galimatron
#include "galimatron/runtime_library/translate/detail/translate_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::runtime
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_trn);
}


void translate_impl_t::logging_to_console_by_default()
{
    use_console();
    fLogmeI("using default logging to console");
}

}// namespace galimatron::runtime
