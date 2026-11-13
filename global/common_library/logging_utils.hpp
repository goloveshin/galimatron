#pragma once

// global
#include "global/common_library/scope_guard.hpp"
#include "global/common_library/type_declaration.hpp"

// Logme
#include <Logme/Logme.h>

// Logme.h defines LS and LWS. In the Galimatron grammar these are ANTLR token names.
#ifdef LS
#undef LS
#endif
#ifdef LWS
#undef LWS
#endif


namespace global::logging_utils
{

// a live object adds 4 spaces to a logme record until the end of the scope.
// indent_backend_t or the channel display-filter inserts the spaces, before the string is formatted.
//
// testing: will (unit)
//
class log_tabulator_t : public scope_guard_t
{
public:
    log_tabulator_t()
        : scope_guard_t(
            []{ global_indent_ += 4; },
            []{ global_indent_ -= 4; }
        )
    {}

    static std::size_t current_indent() { return global_indent_; }

private:
    inline static std::size_t global_indent_ = 0;
};

// default channel, ConsoleBackend
//
// testing: no
//
void use_console();

// named channel with ConsoleBackend and FileBackend, OutputFlags::Channel.
// the calling thread logs there; other threads are forwarded from the default channel.
//
// testing: no
//
void use_named_channel(const std::string& name, const std::string& log_file);

// writes the queued file log. a killed process does not flush on its own
//
// testing: no
//
void flush_log();

// drop backends, then apply native logme JSON
// a relative home-directory.path is resolved from the config file's directory
//
// testing: yes (unit)
//
void apply_log_config(const file_content_with_path_t&);

}// namespace global::logging_utils
