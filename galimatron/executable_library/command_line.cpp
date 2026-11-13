// galimatron
#include "galimatron/executable_library/command_line.hpp"

// Boost
#include <boost/program_options.hpp>

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::executable::command_line
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::exlib);
}


namespace po = boost::program_options;

namespace detail
{

using parsed_options_t = po::basic_parsed_options<char>;
using catch_command_line_and_parse_with_options_ft = std::function<parsed_options_t (const po::options_description&)>;

command_line_args_t parse(catch_command_line_and_parse_with_options_ft parse_func)
{
    constexpr char opt_help[] = "help";
    constexpr char opt_log_config_path[] = "log-config-file";
    constexpr char opt_gal_config_path[] = "gal-config-file";

    po::options_description options("options");
    {
        options.add_options()
            (opt_help, "produce help message")
            (opt_log_config_path, po::value<path_t>(), "boost.log config")
            (opt_gal_config_path, po::value<path_t>(), "galimatron config")
            ;
    }
    po::variables_map vm;
    po::store(parse_func(options), vm);

    command_line_args_t command_line_args;
    {
        const bool is_help = vm.count(opt_help);
        const bool is_help_only = is_help && (vm.size() == 1);

        if (is_help)
        {
            std::stringstream ss;
            ss << options;
            command_line_args.help = ss.str();
        }

        if (!is_help_only)
        {
            po::notify(vm);

            if (const auto& v = vm[opt_log_config_path]; !v.empty())
            {
                command_line_args.log_config_path = v.as<path_t>();
            }
            if (const auto& v = vm[opt_gal_config_path]; !v.empty())
            {
                command_line_args.gal_config_path = v.as<path_t>();
            }
        }
    }
    return command_line_args;
}

}// namespace detail

command_line_args_t parse(const std::string& command_line)
{
    return detail::parse([&](const auto& options)
    {
        return po::command_line_parser(po::split_unix(command_line)).options(options).run();
    });
}

command_line_args_t parse(int argc, char* argv[])
{
    return detail::parse([&](const auto& options)
    {
        return po::parse_command_line(argc, argv, options);
    });
}

}// namespace galimatron::executable::command_line
