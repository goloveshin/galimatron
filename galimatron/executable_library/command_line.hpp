#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::executable::command_line
{

struct command_line_args_t
{
    content_ot help;
    path_ot log_config_path;
    path_ot gal_config_path;
};

// testing: yes (unit)
//
command_line_args_t parse(const std::string& command_line);

// testing: yes (unit)
//
command_line_args_t parse(int argc, char* argv[]);

}// namespace galimatron::executable::command_line
