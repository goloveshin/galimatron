// galimatron
#include "galimatron/runtime_library/translate/detail/translate_impl.hpp"
#include "galimatron/runtime_library/crash/crash.hpp"

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


path_ot translate_impl_t::logging_by_config(const file_content_with_path_ot& cwp)
{
    if (!cwp)
    {
        return {};
    }

    fLogmeI("reconfig logging from config file: '{}'", cwp->path.string());
    apply_log_config(*cwp);
    return crash::arm_log();
}

}// namespace galimatron::runtime
