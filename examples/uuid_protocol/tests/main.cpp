// doctest
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// global
#include "global/common_library/forward.hpp"

// Boost
#include <boost/asio/io_context.hpp>
#include <boost/process/process.hpp>

// uuid_protocol
#include "uuid_protocol/common/client_and_server.hpp"


namespace uuid_protocol
{

namespace
{

// run_tests starts in bin/. a step's client and server are bin/uuid_protocol/steps/<step>/
//
// testing: no
//
std::string executable_in(
    const std::string_view step,
    const std::string_view name
)
{
    auto path = global::fs::path("uuid_protocol") / step / name;
#ifdef _WIN32
    return path.replace_extension(".exe").string();
#else
    return path.string();
#endif
}

// launches the server, then the client. both must exit 0
//
// testing: no
//
void exchange_exits_0(
    const std::string_view step,
    const std::string_view server_name,
    const std::string_view client_name
)
{
    namespace asio = boost::asio;
    namespace bp = boost::process::v2;

    asio::io_context io_context;

    auto server = bp::process(
        io_context,
        executable_in(step, server_name),
        {}
    );
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto client = bp::process(
        io_context,
        executable_in(step, client_name),
        {}
    );

    server.wait();
    client.wait();
    CHECK_EQ(server.exit_code(), 0);
    CHECK_EQ(client.exit_code(), 0);
}

struct history_step_t
{
    std::string folder;
    int index = 0;
};

// the numbered step folders under bin/uuid_protocol/steps, in index order
//
// testing: no
//
std::vector<history_step_t> history_steps()
{
    std::vector<history_step_t> steps;
    const auto root = global::fs::path("uuid_protocol") / "steps";
    for (const auto& entry : global::fs::directory_iterator(root))
    {
        if (!entry.is_directory())
        {
            continue;
        }
        const auto name = entry.path().filename().string();
        std::size_t used = 0;
        const auto index = std::stoi(name, &used);
        if (used == 0 || name.at(used) != '-')
        {
            continue;
        }
        steps.push_back({
            .folder = name,
            .index = index
        });
    }
    steps |= ranges::actions::sort(ranges::less{}, &history_step_t::index);
    return steps;
}

// one client, already aimed at the shared port. returns its exit code
//
// testing: no
//
int client_exit_code(
    const std::string_view step,
    const std::string_view client_name,
    const std::vector<std::string>& args
)
{
    namespace asio = boost::asio;
    namespace bp = boost::process::v2;

    asio::io_context io_context;
    auto client = bp::process(
        io_context,
        executable_in(step, client_name),
        args
    );
    return client.wait();
}

// a cross-test log, under bin/uuid_protocol/cross/.
// role distinguishes the file, for example client-on-11 or server-for-3
//
// testing: no
//
std::string cross_log(
    const int index,
    const std::string_view role
)
{
    return (
        global::fs::path("uuid_protocol")
            / "cross"
            / std::format("uuid_protocol_{}_{}.log", index, role)
    ).string();
}

// a step whose folder contains "collapse" speaks one version.
// the converting server is the step before that. with no collapse step, it is the last step.
//
// testing: no
//
history_step_t converting_step(const std::vector<history_step_t>& steps)
{
    const auto collapse = ranges::find_if(steps, [](const history_step_t& step)
    {
        return boost::contains(step.folder, "collapse");
    });
    if (collapse == steps.begin() || collapse == steps.end())
    {
        return steps.back();
    }
    return *std::prev(collapse);
}

// runs every client against this server.
// a client that does not reach it exits non-zero, and that refusal stops the server.
// the next client starts the server again.
// a server left accepting is ended by an empty connection and exits 0
//
// testing: no
//
void every_client_against(
    const std::vector<history_step_t>& steps,
    const history_step_t& server_step,
    // true when this client speaks a uuid this server registers
    const std::function<bool (const history_step_t&)>& reaches
)
{
    namespace asio = boost::asio;
    namespace bp = boost::process::v2;

    asio::io_context io_context;
    std::optional<bp::process> server;

    // terminates the server when the case leaves it running
    //
    struct stop_server_t
    {
        std::optional<bp::process>& server;

        ~stop_server_t()
        {
            if (server && server->running())
            {
                server->terminate();
                server->wait();
            }
        }
    };
    const stop_server_t stop{ server };

    auto launch = [&](const history_step_t& client_step)
    {
        server.emplace(
            io_context,
            executable_in(
                "steps/" + server_step.folder,
                std::format("uuid_protocol_{}_server", server_step.index)
            ),
            std::vector<std::string>{
                "--accept-until-stopped",
                "--log-file",
                cross_log(server_step.index, std::format("server-for-{}", client_step.index))
            }
        );
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    for (const auto& client_step : steps)
    {
        if (!server || !server->running())
        {
            launch(client_step);
        }

        const auto where = std::format(
            "client {} against server {}",
            client_step.index,
            server_step.index
        );
        const auto code = client_exit_code(
            "steps/" + client_step.folder,
            std::format("uuid_protocol_{}_client", client_step.index),
            {
                "--log-file",
                cross_log(client_step.index, std::format("client-on-{}", server_step.index))
            }
        );
        const auto client_reaches = reaches(client_step);
        CHECK_MESSAGE(client_reaches ? code == 0 : code != 0, where);
        if (!client_reaches)
        {
            CHECK_MESSAGE(server->wait() != 0, where);
        }
        else if (!server->running())
        {
            CHECK_MESSAGE(server->wait() == 0, where);
        }
    }

    if (server && server->running())
    {
        asio::ip::tcp::socket nudge(io_context);
        nudge.connect(
            asio::ip::tcp::endpoint(
                asio::ip::make_address("127.0.0.1"),
                client_and_server_t::port
            )
        );
        nudge.shutdown(asio::ip::tcp::socket::shutdown_both);
        nudge.close();
        CHECK_EQ(server->wait(), 0);
    }
}

}

TEST_SUITE("uuid_protocol/steps/00-no-dto-hello-ok-bye")
{

// there is no dto. the exchange is hello, then ok, then bye
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/00-no-dto-hello-ok-bye",
        "uuid_protocol_0_server",
        "uuid_protocol_0_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/01-empty-dto-hello-ok-bye")
{

// the exchange sends the generated hello_t, ok_t, and bye_t
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/01-empty-dto-hello-ok-bye",
        "uuid_protocol_1_server",
        "uuid_protocol_1_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/02-unpublished-version-hello-ok-bye")
{

// version 1 is not published, so the exchange is still the generated v1 structs
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/02-unpublished-version-hello-ok-bye",
        "uuid_protocol_2_server",
        "uuid_protocol_2_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/03-expand-hello")
{

// v2 hello_t carries message. the client sends "hello client version v2!"
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/03-expand-hello",
        "uuid_protocol_3_server",
        "uuid_protocol_3_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/04-prepare-payloads")
{

// v2 types are in namespace system. the client hello is "hello client version v2!"
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/04-prepare-payloads",
        "uuid_protocol_4_server",
        "uuid_protocol_4_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/05-start-payloads")
{

// v2 hello is "hello client version v2!". v3 hello is "hello client version v3!", then lamp_t{ lit: true }
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/05-start-payloads",
        "uuid_protocol_5_server",
        "uuid_protocol_5_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/06-state-conversions")
{

// the exchange is the step 5 exchange. generation adds the conversion templates
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/06-state-conversions",
        "uuid_protocol_6_server",
        "uuid_protocol_6_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/07-candela-and-fog")
{

// v4 hello is "hello client version v4!", then lamp_t{ lit: true, candela: 1500 } and fog_t{ metres: 250 }
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/07-candela-and-fog",
        "uuid_protocol_7_server",
        "uuid_protocol_7_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/08-sea-and-vessel")
{

// v5 hello is "hello client version v5!", then lamp, fog with sea swell, and vessel
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/08-sea-and-vessel",
        "uuid_protocol_8_server",
        "uuid_protocol_8_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/09-watch")
{

// v6 hello is "hello client version v6!", then watch and vessel
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/09-watch",
        "uuid_protocol_9_server",
        "uuid_protocol_9_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/11-substitution")
{

// the exchange is the step 10 exchange. the dictionary greeting is rendered text
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/11-substitution",
        "uuid_protocol_11_server",
        "uuid_protocol_11_client"
    );
}

}

TEST_SUITE("uuid_protocol/steps/12-collapse")
{

// the only published version is v6. hello, the watch, the vessel, then bye
//
TEST_CASE("client connects and disconnects")
{
    exchange_exits_0(
        "steps/12-collapse",
        "uuid_protocol_12_server",
        "uuid_protocol_12_client"
    );
}

}

TEST_SUITE("uuid_protocol/cross")
{

// the converting server stays up across clients it can read.
// step 0 registers 6f1e0000 uuids, so that client and this server both exit non-zero.
// a client after this server speaks the uuid advanced past it, so that pair exits non-zero too.
// every other client exits 0. logs are uuid_protocol/cross/, so a paired run keeps the step logs.
//
TEST_CASE("every client meets the converting server")
{
    const auto steps = history_steps();
    REQUIRE(steps.size() >= 2);

    const auto converting = converting_step(steps);
    every_client_against(
        steps,
        converting,
        [&](const history_step_t& client)
        {
            return client.index != 0 && client.index <= converting.index;
        }
    );
}

// the server after the converting step registers the surviving version only.
// the client of that same step exits 0.
// every earlier hello uuid is unknown, so that client and the server both exit non-zero.
//
TEST_CASE("every client meets the server after the converting step")
{
    const auto steps = history_steps();
    const auto converting = converting_step(steps);
    const auto converting_at = ranges::find(steps, converting.index, &history_step_t::index);
    REQUIRE(converting_at != steps.end());
    REQUIRE(std::next(converting_at) != steps.end());
    const auto& collapsed = *std::next(converting_at);

    every_client_against(
        steps,
        collapsed,
        [&](const history_step_t& client)
        {
            return client.index == collapsed.index;
        }
    );
}

}

}// namespace uuid_protocol
