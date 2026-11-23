// galimatron
#include "galimatron/runtime_library/translate/translate.hpp"
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

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


namespace ex = galimatron::executable;

galimatron_config::config_t translate_t::get_gal_config_from_file(const path_t& config_path)
{
    auto filesystem = ex::filesystem::make_filesystem();
    // the config reads xslt later, after this filesystem is gone
    const auto read = filesystem.read;

    return galimatron_config::read_from_json(
        {
            .content = make_file_content(read(config_path)),
            .path = filesystem.absolute(config_path)
        },
        [read](const path_t& path)
        {
            return make_file_content(read(path));
        }
    );
}

}// namespace galimatron::runtime
