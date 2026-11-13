#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"
#include "uuid_protocol/common/protocol_forward.hpp"

// Boost
#include <boost/asio.hpp>


namespace uuid_protocol
{

// frame: uuid, uint32 length, then the body when the message has one.
// no test cases: one connection, hello then ok then bye.
// test cases: one connection per version. hello, each case, then bye.
//
class client_and_server_t
{
public:
    using tcp = boost::asio::ip::tcp;

    // uuid_protocol listens on 5556. every history step listens on this port
    inline static constexpr boost::asio::ip::port_type port = 5557;
    inline static constexpr std::uint32_t message_max_size = 1024;

    // opens the log named in params
    //
    // testing: no
    //
    struct params_t
    {
        std::string log_prefix;
        std::string log_file_name;
        boost::asio::ip::port_type port;
        state_operations_t state_operations;
        version_test_cases_t version_test_cases;
        // the cross-test sets this. the server then accepts until the process is stopped
        bool accept_until_stopped = false;
    };
    explicit client_and_server_t(params_t params);

    // false: the session threw. the exception is logged
    //
    // testing: no
    //
    bool run_client();

    // false: the session threw. the exception is logged
    //
    // testing: no
    //
    bool run_server();

private:
    // false: the job threw. the exception is logged
    //
    // testing: no
    //
    bool run(const std::function<void ()>& job);

    // hello, then ok, then bye
    //
    // testing: no
    //
    void run_client_sessions();

    // one connection per test case. accept_until_stopped keeps accepting until the process is stopped
    //
    // testing: no
    //
    void run_server_sessions();

    params_t params_;
};

}// namespace uuid_protocol
