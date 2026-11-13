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


void translate_impl_t::translate_with_time(const galimatron_config::config_t& config)
{
    auto timing = scope_guard_t(
        [&]
        {
            fLogmeI("begin of Galimatron");
        },
        [&](scope_guard_t& timing)
        {
            const auto suffix = [&] () -> std::string
            {
                return config.galimatron.committed_report
                    ? ""
                    : std::format(": {}", timing.duration().text());
            }();
            fLogmeI("end of Galimatron{}", suffix);
        }
    );

    translate(config);
}

}// namespace galimatron::runtime
