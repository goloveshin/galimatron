#pragma once

// global
#include "global/common_library/forward.hpp"


namespace uuid_protocol
{

using namespace global::logging_utils;

using uuid_t = boost::uuids::uuid;
using uuid_gen_t = boost::uuids::string_generator;
inline uuid_gen_t uuid_gen;

using any_state_t = std::any;
using any_message_t = std::any;

using group_t = int;
using version_t = int;

using buffer_t = std::vector<std::uint8_t>;

// a message is a group. the body is empty unless the operation writes one
//
struct message_operations_t
{
    using handle_ft = std::function<void (const any_message_t&, const any_state_t&)>;
    using log_ft = std::function<void (const any_message_t&)>;
    using write_body_ft = std::function<buffer_t (const any_message_t&)>;
    using read_body_ft = std::function<any_message_t (const buffer_t&)>;
    using create_ft = std::function<any_message_t (const any_state_t&)>;

    uuid_t uuid;
    version_t version;
    group_t group;
    handle_ft handle_client;
    handle_ft handle_server;
    log_ft log;
    write_body_ft write_body;
    read_body_ft read_body;
    create_ft create;
};

struct state_operations_t
{
    using io_notify_ft = std::function<void (const any_state_t&, const std::string&)>;

    any_state_t any_state;
    io_notify_ft io_notify;
};

// one entry is one version and the messages the client sends after hello
//
struct version_test_case_t
{
    struct test_case_t
    {
        uuid_t uuid;
        any_message_t any_message;
    };
    using test_case_ft = std::function<test_case_t ()>;
    using test_cases_t = std::vector<test_case_ft>;

    version_t version;
    test_cases_t test_cases;
};
using version_test_cases_t = std::vector<version_test_case_t>;

}// namespace uuid_protocol
