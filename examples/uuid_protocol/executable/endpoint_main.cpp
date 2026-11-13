// uuid_protocol
#include "uuid_protocol/common/client_and_server.hpp"
#include "uuid_protocol/common/protocol.hpp"
#include "uuid_protocol/common/state.hpp"
#include "uuid_protocol/common/test_case_container.hpp"

// Boost
#include <boost/program_options.hpp>

// the target sets two macros:
//     UUID_PROTOCOL_IS_CLIENT     1 for the client, 0 for the server
//     UUID_PROTOCOL_LOG_FILE      path relative to bin/
// UUID_PROTOCOL_PORT is set for a step that does not listen on client_and_server_t::port
// options:
//     --help                      produce help message
//     --accept-until-stopped      the server keeps accepting
//     --log-file <path>           write this file instead of UUID_PROTOCOL_LOG_FILE

#ifndef UUID_PROTOCOL_PORT
#define UUID_PROTOCOL_PORT client_and_server_t::port
#endif


namespace
{

namespace po = boost::program_options;

struct endpoint_args_t
{
    bool help = false;
    bool accept_until_stopped = false;
    global::name_ot log_file;
};

// --help prints the options and sets help. the other options are then ignored.
// an unknown argument, or --log-file without a path, throws boost::program_options::error
//
// testing: no
//
endpoint_args_t parse_args(int argc, char* argv[])
{
    constexpr char opt_help[] = "help";
    constexpr char opt_accept[] = "accept-until-stopped";
    constexpr char opt_log_file[] = "log-file";

    po::options_description options("options");
    {
        options.add_options()
            (opt_help, "produce help message")
            (opt_accept, "the server keeps accepting")
            (opt_log_file, po::value<global::name_t>(), "write this file instead of the compiled log path")
            ;
    }

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);

    endpoint_args_t args;
    args.help = vm.count(opt_help) != 0;
    if (args.help)
    {
        std::cout << options << std::endl;
        return args;
    }

    po::notify(vm);
    args.accept_until_stopped = vm.count(opt_accept) != 0;
    if (const auto& v = vm[opt_log_file]; !v.empty())
    {
        args.log_file = v.as<global::name_t>();
    }
    return args;
}

}// namespace


int main(int argc, char* argv[])
{
    using namespace uuid_protocol;

    const auto args = parse_args(argc, argv);
    if (args.help)
    {
        return 0;
    }

    registrate();
    registrate_test_cases();

    auto endpoint = client_and_server_t(
    {
        .log_prefix = UUID_PROTOCOL_IS_CLIENT ? "client" : "server",
        .log_file_name = args.log_file.value_or(UUID_PROTOCOL_LOG_FILE),
        .port = UUID_PROTOCOL_PORT,
        .state_operations = UUID_PROTOCOL_IS_CLIENT ? make_client_state() : make_server_state(),
        .version_test_cases = test_cases(),
        .accept_until_stopped = args.accept_until_stopped
    });

    const auto ok = UUID_PROTOCOL_IS_CLIENT
        ? endpoint.run_client()
        : endpoint.run_server();
    return ok ? 0 : 1;
}
