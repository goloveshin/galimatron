// galimatron
#include "galimatron/runtime_library/translate/detail/translate_impl.hpp"
#include "galimatron/runtime_library/crash/crash.hpp"
#include "galimatron/runtime_library/leaf_spy/leaf_spy.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"

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


translate_impl_t::translate_impl_t(const translate_t& args)
    : args_(args)
{
}

int translate_impl_t::translate()
{
    crash::install();

    // leaf exceptions are logged to the console at least
    logging_to_console_by_default();

    int result = 1;

    // empty until the job has a gal.cfg. the guard outlives the catch, which reads e_path_info only
    const auto report_guard = galimatron::path_report_guard_t({});

    runtime::try_catch({
        .global_args = {
            .job_that_may_throw = [&]
            {
                const auto crash_log = logging_by_config(args_.get_log_config());
                const auto gal_config = args_.get_gal_config();
                galimatron::current_path_report() = {
                    .committed_report = gal_config.galimatron.committed_report,
                    .config_dir = gal_config.config_dir
                };
                if (crash_log)
                {
                    fLogmeI(
                        "crash log: '{}'",
                        translator::galimatron_config::report_path(gal_config, *crash_log)
                    );
                }
                translate_with_time(gal_config);
            },
            .notify_that_job_executed_with_throw = [&]
            {
                result = 1;
            },
            .notify_that_job_executed_without_throw = [&]
            {
                result = 0;
            },
            .is_throw_out = args_.is_testing_mode
        },
        .notify_printed = [](const std::string& info)
        {
            fLogmeE("{}", info);
        }
    });

    // the file backend queues lines. the log file is complete when translate returns
    flush_log();

    return result;
}

int translate(const translate_t& args)
{
    return translate_impl_t(args).translate();
}

}// namespace galimatron::runtime
