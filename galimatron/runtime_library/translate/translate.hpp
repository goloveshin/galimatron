#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"
#include "galimatron/runtime_library/galimatron_config/galimatron_config.hpp"
#include "galimatron/translator_library/translator/translator.hpp"


namespace galimatron::runtime
{

struct translate_t
{
    using get_log_config_ft = std::function<file_content_with_path_ot ()>;
    using get_gal_config_ft = std::function<galimatron_config::config_t ()>;

    // at the moment of the call some logging will already be available
    get_log_config_ft get_log_config;
    get_gal_config_ft get_gal_config;
    // testing mode - exceptions leave translate()
    const bool is_testing_mode = true;

    // absent: the production pipeline. set: the chosen builtin steps
    std::optional<translator::translator::detail::testing_t> testing;

    // empty: read and write keep the bytes. tests pass the pugixml round-trip
    gf::abstract::content_filter_ft content_filter = {};

    static file_content_with_path_t get_log_config_from_file(const path_t&);
    static galimatron_config::config_t get_gal_config_from_file(const path_t&);
};
int translate(const translate_t&);

}// namespace galimatron::runtime
