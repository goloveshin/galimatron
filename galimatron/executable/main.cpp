// galimatron
#include "galimatron/executable_library/command_line.hpp"
#include "galimatron/runtime_library/translate/translate.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


int main(int argc, char* argv[])
{
    LOGME_SUBSYSTEM(SUBSID, global::log_subsystems::exec);

    namespace glb = global;
    namespace ex = galimatron::executable;
    namespace rt = galimatron::runtime;

    const auto [help, log_config_path, gal_config_path] = ex::command_line::parse(argc, argv);

    if (help)
    {
        std::cout << *help << std::endl;
        return 0;
    }

    return rt::translate(
    {
        .get_log_config = [&]
        {
            // an empty path keeps the console log already installed
            return log_config_path.transform([](const glb::path_t& path)
            {
                return rt::translate_t::get_log_config_from_file(path);
            });
        },
        .get_gal_config = [&]
        {
            return gal_config_path.transform([](const glb::path_t& path)
            {
                return rt::translate_t::get_gal_config_from_file(path);
            })
            .or_else([]() -> std::optional<rt::galimatron_config::config_t>
            {
                glb::leaf::throw_exception(
                    exception::domain::executable{},
                    exception::e_what_info{ "must have 'gal-config-file' in command line" }
                );
                return {};
            })
            .value();
        },
        .is_testing_mode = false
    });
}
