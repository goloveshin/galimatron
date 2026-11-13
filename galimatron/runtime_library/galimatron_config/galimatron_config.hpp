#pragma once

// galimatron
#include "galimatron/translator_library/galimatron_config_declaration.hpp"


namespace galimatron::runtime::galimatron_config
{

using config_t = translator::galimatron_config::config_t;

// testing: yes (unit)
//
config_t read_from_json(
    const file_content_with_path_t&,
    // the file-reading function that will be used
    // to create deferred_content in config_t
    const global::filesystem::abstract::read_ft&
);

// testing: yes (unit)
//
content_t write_to_json(const config_t&);

}// namespace galimatron::runtime::galimatron_config
