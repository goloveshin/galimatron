// galimatron
#include "galimatron/executable_library/command_line.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

namespace cl = executable::command_line;

// Google: 'c++ convert vector to argc argv'
//
cl::command_line_args_t convert_vector_to_argc_argv
(
    const std::list<std::string>& const_args,
    const std::function<cl::command_line_args_t (int /* argc */, char* /* argv */[])>& parse_func
)
{
    auto args = const_args;
    {
        args.push_front("galimatron_exe");
    }

    // 1. Create a vector of char* to hold the pointers
    std::vector<char*> argv;
    argv.reserve(args.size() + 1); // +1 for the terminating null

    // 2. Point to the internal data of the std::string
    // Note: The strings must remain in memory as long as argv is used
    for (auto& s : args)
    {
        argv.push_back(s.data());
    }

    // 3. Null-terminate the array (required by C standards)
    argv.push_back(nullptr);

    // 4. Extract argc and argv
    int argc = static_cast<int>(args.size());
    char** argv_ptr = argv.data();

    return parse_func(argc, argv_ptr);
}

TEST_SUITE("galimatron/executable_library/command_line" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("parse")
{
    for (const bool parse_variant : std::vector<bool>{ true, false })
    {
        auto parse = [&](const std::list<std::string>& strings)
        {
            if (parse_variant)
            {
                return cl::parse(boost::algorithm::join(strings, " "));
            }
            else
            {
                return convert_vector_to_argc_argv(
                    strings,
                    [](int argc, char* argv[]){ return cl::parse(argc, argv); }
                );
            }
        };

        SUBCASE("--help")
        {
            const auto actual = parse(
            {
                "--help"
            });

            CHECK(actual.help.has_value());
            CHECK_FALSE(actual.log_config_path.has_value());
            CHECK_FALSE(actual.gal_config_path.has_value());
        }

        SUBCASE("--log-config-file")
        {
            const auto actual = parse(
            {
                "--log-config-file", "/root/home/user/project/log.cfg"
            });

            CHECK_FALSE(actual.help.has_value());
            CHECK(actual.log_config_path == path_ot("/root/home/user/project/log.cfg"));
            CHECK_FALSE(actual.gal_config_path.has_value());
        }

        SUBCASE("--gal-config-file")
        {
            const auto actual = parse(
            {
                "--gal-config-file", "/root/home/user/project/gal.cfg"
            });

            CHECK_FALSE(actual.help.has_value());
            CHECK_FALSE(actual.log_config_path.has_value());
            CHECK(actual.gal_config_path == path_ot("/root/home/user/project/gal.cfg"));
        }

        SUBCASE("exception on bad command line, fail")
        {
            CHECK_THROWS_WITH(
                parse(
                {
                    "--log-config-filee", "/root/home/user/project/log.cfg"
                }),
                "unrecognised option '--log-config-filee'"
            );
        }
    };
}

}

}// namespace galimatron
