// uuid_protocol
#include "uuid_protocol/common/client_and_server.hpp"
#include "uuid_protocol/common/message_container.hpp"


namespace uuid_protocol
{

namespace
{

// the name of a system group, or empty when the group is not one
//
// testing: no
//
std::string_view system_message_name(const group_t group)
{
    switch (group)
    {
    case hello_group: return "hello";
    case bye_group: return "bye";
    case ok_group: return "ok";
    case error_group: return "error";
    default: return {};
    }
}

// the direction and the group, then the body from operations.log
//
// testing: no
//
void log_exchange(
    const std::string_view direction,
    const message_operations_t& operations,
    const any_message_t& message
)
{
    if (operations.group == no_group)
    {
        fLogmeI("{} user", direction);
    }
    else
    {
        fLogmeI(
            "{} system {}",
            direction,
            system_message_name(operations.group)
        );
    }
    const auto tab = log_tabulator_t();
    operations.log(message);
    // the file backend queues the line. flush it before the next read, so a killed process still has it
    flush_log();
}

// one frame after it has been read. operations is null when the uuid is not registered
//
struct incoming_t
{
    uuid_t uuid{};
    const message_operations_t* operations = nullptr;
    any_message_t message;
};

// reads one frame: uuid, uint32 length, then the body.
// a body is accepted only when the message reads one.
// an unknown uuid leaves operations null and is not logged.
// throws std::runtime_error when the length is above message_max_size,
// or when bytes arrive and the message has no read_body
//
// testing: no
//
incoming_t read_message(client_and_server_t::tcp::socket& socket)
{
    incoming_t incoming;
    boost::asio::read(
        socket,
        boost::asio::buffer(&incoming.uuid, sizeof(incoming.uuid))
    );

    std::uint32_t incoming_size = 0;
    boost::asio::read(
        socket,
        boost::asio::buffer(&incoming_size, sizeof(incoming_size))
    );
    if (incoming_size > client_and_server_t::message_max_size)
    {
        throw std::runtime_error(
            std::format(
                "message length {} is greater than {}",
                incoming_size,
                client_and_server_t::message_max_size
            )
        );
    }

    buffer_t buffer(incoming_size);
    if (incoming_size != 0)
    {
        boost::asio::read(
            socket,
            boost::asio::buffer(buffer)
        );
    }

    const auto found = messages().try_find_by_uuid(incoming.uuid);
    if (!found)
    {
        return incoming;
    }
    incoming.operations = &found->get();
    if (incoming.operations->read_body)
    {
        incoming.message = incoming.operations->read_body(buffer);
    }
    else if (incoming_size != 0)
    {
        throw std::runtime_error(
            std::format(
                "a message has no body, length is {}",
                incoming_size
            )
        );
    }
    log_exchange("<-", *incoming.operations, incoming.message);
    return incoming;
}

// the cross-test closes a connection that sends no bytes. that ends the server.
// any other failure still throws.
//
// testing: no
//
incoming_t read_message_or_end(
    client_and_server_t::tcp::socket& socket,
    // true only before hello, and only while the server is accepting until stopped
    const bool end_on_close,
    bool& ended
)
{
    try
    {
        return read_message(socket);
    }
    catch (const boost::system::system_error& error)
    {
        const auto code = error.code();
        if (
            end_on_close
            && (code == boost::asio::error::eof || code == boost::asio::error::connection_reset)
        )
        {
            ended = true;
            return {};
        }
        throw;
    }
}

// writes one frame: uuid, uint32 length, then the body.
// the body is empty unless the message writes one.
// throws std::runtime_error when the body is above message_max_size
//
// testing: no
//
void write_message(
    client_and_server_t::tcp::socket& socket,
    const message_operations_t& operations,
    const any_message_t& message
)
{
    log_exchange("->", operations, message);

    buffer_t buffer;
    if (operations.write_body)
    {
        buffer = operations.write_body(message);
    }
    if (buffer.size() > client_and_server_t::message_max_size)
    {
        throw std::runtime_error(
            std::format(
                "message length {} is greater than {}",
                buffer.size(),
                client_and_server_t::message_max_size
            )
        );
    }
    const auto buffer_size = static_cast<std::uint32_t>(buffer.size());
    boost::asio::write(
        socket,
        boost::asio::buffer(&operations.uuid, sizeof(operations.uuid))
    );
    boost::asio::write(
        socket,
        boost::asio::buffer(&buffer_size, sizeof(buffer_size))
    );
    if (buffer_size != 0)
    {
        boost::asio::write(
            socket,
            boost::asio::buffer(buffer)
        );
    }
}

// sends operations.create(state) when it is set, otherwise an empty value.
// state is the text io_notify stored, which error create copies into the message
//
// testing: no
//
void write_created(
    client_and_server_t::tcp::socket& socket,
    const message_operations_t& operations,
    const any_state_t& state
)
{
    any_message_t message;
    if (operations.create)
    {
        message = operations.create(state);
    }
    write_message(socket, operations, message);
}

// sends the first registered message of this group.
// a later version of the same group is not selected.
// throws std::logic_error when the group is not registered
//
// testing: no
//
void write_group(
    client_and_server_t::tcp::socket& socket,
    const group_t group,
    const any_state_t& state
)
{
    write_created(socket, messages().find_by_group(group), state);
}

// sends the message registered for this version and group.
// throws std::logic_error when that pair is not registered
//
// testing: no
//
void write_group(
    client_and_server_t::tcp::socket& socket,
    const version_t version,
    const group_t group,
    const any_state_t& state
)
{
    write_created(socket, messages().find_by_version_and_group(version, group), state);
}

// the highest version this endpoint registered. no messages: std::logic_error
//
// testing: no
//
version_t last_version()
{
    auto& records = messages();
    if (records.empty())
    {
        throw std::logic_error("no messages");
    }
    return ranges::max(records | ranges::views::transform(&message_operations_t::version));
}

}// namespace

// opens the log named in params
//
// testing: no
//
client_and_server_t::client_and_server_t(params_t params)
    : params_(std::move(params))
{
    use_named_channel(params_.log_prefix, params_.log_file_name);
}

// false: the job threw. the exception text is logged, then "stop"
//
// testing: no
//
bool client_and_server_t::run(const std::function<void ()>& job)
{
    fLogmeI("start");
    try
    {
        fLogmeI("last version: {}", last_version());
        flush_log();
        job();
    }
    catch (const std::exception& e)
    {
        fLogmeE("{}", e.what());
        fLogmeI("stop");
        flush_log();
        return false;
    }
    fLogmeI("stop");
    flush_log();
    return true;
}

// false: the session threw. the exception is logged
//
// testing: no
//
bool client_and_server_t::run_client()
{
    return run([&]{ run_client_sessions(); });
}

// false: the session threw. the exception is logged
//
// testing: no
//
bool client_and_server_t::run_server()
{
    return run([&]{ run_server_sessions(); });
}

// no cases: one connection, the first registered hello, then ok, then bye.
// cases: one connection per version. hello, each case value, then bye.
// throws std::runtime_error when a reply uuid is unknown, the group is not ok,
// or, once a version is in use, the reply version differs
//
// testing: no
//
void client_and_server_t::run_client_sessions()
{
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    const auto endpoints = resolver.resolve(
        tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), params_.port)
    );

    // one socket per exchange
    auto connect = [&]()
    {
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        return socket;
    };

    // the reply must be ok. expected, when set, is the only accepted version
    auto require_ok = [&](
        tcp::socket& socket,
        // empty: do not check the reply version
        const std::optional<version_t> expected
    )
    {
        const auto incoming = read_message(socket);
        if (!incoming.operations)
        {
            throw std::runtime_error(
                std::format(
                    "no message with uuid: {}",
                    boost::uuids::to_string(incoming.uuid)
                )
            );
        }

        const auto& operations = *incoming.operations;
        if (expected && operations.version != *expected)
        {
            throw std::runtime_error(
                std::format(
                    "version of message and protocol are differ: {}:{}",
                    operations.version,
                    *expected
                )
            );
        }
        operations.handle_client(incoming.message, params_.state_operations.any_state);
        if (operations.group != ok_group)
        {
            throw std::runtime_error("client expects 'ok'");
        }
    };

    // the first registered hello, ok, and bye. later versions of those groups stay unused
    if (params_.version_test_cases.empty())
    {
        auto socket = connect();
        write_group(socket, hello_group, params_.state_operations.any_state);
        require_ok(socket, {});
        write_group(socket, bye_group, params_.state_operations.any_state);
        socket.close();
        return;
    }

    // each entry is its own connection, so a later version does not continue an earlier one
    for (const auto& version_test_case : params_.version_test_cases)
    {
        const auto current_version = version_test_case.version;
        fLogmeI("start exchange at version: {}", current_version);

        auto socket = connect();
        write_group(socket, current_version, hello_group, params_.state_operations.any_state);

        std::size_t index = 0;
        for (;;)
        {
            require_ok(socket, current_version);

            if (index < version_test_case.test_cases.size())
            {
                const auto test_case = version_test_case.test_cases[index++]();
                // the case value is the body. a default object is not sent
                write_message(
                    socket,
                    messages().find_by_uuid(test_case.uuid),
                    test_case.any_message
                );
                continue;
            }

            write_group(socket, current_version, bye_group, params_.state_operations.any_state);
            socket.close();
            break;
        }
    }
}

// no cases: one connection. cases: one connection per version.
// accept_until_stopped ignores that count and keeps accepting until the process is stopped.
// the first message sets the version and must be hello.
// later messages must be that version. bye ends the connection.
// a payload is answered with that version's ok. another system group is refused.
// throws std::runtime_error on those failures.
// an unknown uuid throws too, and an error is sent only after a version is known
//
// testing: no
//
void client_and_server_t::run_server_sessions()
{
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), params_.port));

    // records the refusal reason on the endpoint state. the caller then sends that version's error
    auto notify = [&](const std::string& msg)
    {
        params_.state_operations.io_notify(params_.state_operations.any_state, msg);
    };

    // the client opens one connection when it has no cases, otherwise one per version
    const auto session_count = params_.version_test_cases.empty()
        ? 1
        : static_cast<int>(params_.version_test_cases.size());

    for (
        int session = 0;
        params_.accept_until_stopped || session < session_count;
        ++session
    )
    {
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        // unset until hello names the version for the rest of this connection
        std::optional<version_t> current_version;
        for (;;)
        {
            bool ended = false;
            const auto incoming = read_message_or_end(
                socket,
                params_.accept_until_stopped && !current_version,
                ended
            );
            if (ended)
            {
                return;
            }
            if (!incoming.operations)
            {
                // an error reply needs the session version. before hello there is none
                if (current_version)
                {
                    notify("no such message");
                    write_group(socket, *current_version, error_group, params_.state_operations.any_state);
                }
                throw std::runtime_error(
                    std::format(
                        "no message with uuid: {}",
                        boost::uuids::to_string(incoming.uuid)
                    )
                );
            }

            const auto& operations = *incoming.operations;
            // hello of this message's version starts the exchange
            if (!current_version)
            {
                current_version = operations.version;
                if (operations.group != hello_group)
                {
                    notify("first message must be 'hello'");
                    write_group(socket, *current_version, error_group, params_.state_operations.any_state);
                    throw std::runtime_error("first message must be 'hello'");
                }
                fLogmeI("start exchange at version: {}", *current_version);
                operations.handle_server(incoming.message, params_.state_operations.any_state);
                write_group(socket, *current_version, ok_group, params_.state_operations.any_state);
                continue;
            }

            if (operations.version != *current_version)
            {
                notify("version of all messages must be same");
                write_group(socket, *current_version, error_group, params_.state_operations.any_state);
                throw std::runtime_error(
                    std::format(
                        "version of message and protocol are differ: {}:{}",
                        operations.version,
                        *current_version
                    )
                );
            }

            operations.handle_server(incoming.message, params_.state_operations.any_state);
            if (operations.group == bye_group)
            {
                break;
            }
            // hello was the first message. a later system message is not a payload
            if (operations.group != no_group)
            {
                notify("unexpected system message");
                write_group(socket, *current_version, error_group, params_.state_operations.any_state);
                throw std::runtime_error("unexpected system message");
            }
            write_group(socket, *current_version, ok_group, params_.state_operations.any_state);
        }

        socket.close();
    }
}

}// namespace uuid_protocol
