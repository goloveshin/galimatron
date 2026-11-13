#pragma once

// galimatron
#include "galimatron/runtime_library/translate/translate.hpp"


namespace galimatron::runtime
{

class translate_impl_t
{
public:
    translate_impl_t(const translate_t&);

    translate_impl_t(const translate_impl_t&) = delete;
    translate_impl_t& operator=(const translate_impl_t&) = delete;
    translate_impl_t(translate_impl_t&&) = delete;
    translate_impl_t& operator=(translate_impl_t&&) = delete;

public:
    int translate();

private:
    void logging_to_console_by_default();
    path_ot logging_by_config(const file_content_with_path_ot&);
    void translate_with_time(const galimatron_config::config_t&);
    void translate(const galimatron_config::config_t&);

private:
    const translate_t& args_;
};

}// namespace galimatron::runtime
