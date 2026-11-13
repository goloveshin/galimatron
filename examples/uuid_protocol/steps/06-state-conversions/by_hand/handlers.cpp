// uuid_protocol
#include "uuid_protocol/common/protocol.hpp"
#include "v1/v1.hpp"
#include "v2/v2.hpp"
#include "v3/v3.hpp"


namespace uuid_protocol::v1
{

// testing: no
//
any_state_t create(std::type_identity<bye_t>, const any_state_t&)
{
    return bye_t{};
}

// testing: no
//
void log(const bye_t&)
{
    fLogmeI("bye_t{{}}");
}

// testing: no
//
void handle_server(const bye_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const bye_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<error_t>, const any_state_t&)
{
    return error_t{};
}

// testing: no
//
void log(const error_t&)
{
    fLogmeI("error_t{{}}");
}

// testing: no
//
void handle_server(const error_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const error_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<hello_t>, const any_state_t&)
{
    return hello_t{};
}

// testing: no
//
void log(const hello_t&)
{
    fLogmeI("hello_t{{}}");
}

// testing: no
//
void handle_server(const hello_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const hello_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<ok_t>, const any_state_t&)
{
    return ok_t{};
}

// testing: no
//
void log(const ok_t&)
{
    fLogmeI("ok_t{{}}");
}

// testing: no
//
void handle_server(const ok_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const ok_t&, const any_state_t&)
{
}

}// namespace uuid_protocol::v1

namespace uuid_protocol::v2::system
{

// testing: no
//
any_state_t create(std::type_identity<bye_t>, const any_state_t&)
{
    return bye_t{};
}

// testing: no
//
void log(const bye_t&)
{
    fLogmeI("bye_t{{}}");
}

// testing: no
//
void handle_server(const bye_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const bye_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<error_t>, const any_state_t&)
{
    return error_t{};
}

// testing: no
//
void log(const error_t&)
{
    fLogmeI("error_t{{}}");
}

// testing: no
//
void handle_server(const error_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const error_t&, const any_state_t&)
{
}

// the client hello names this version. v1 hello has no message field
//
// testing: no
//
any_state_t create(std::type_identity<hello_t>, const any_state_t&)
{
    return hello_t{ .message = "hello client version v2!" };
}

// testing: no
//
void log(const hello_t& value)
{
    fLogmeI("hello_t{{{}}}", value.message);
}

// testing: no
//
void handle_server(const hello_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const hello_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<ok_t>, const any_state_t&)
{
    return ok_t{};
}

// testing: no
//
void log(const ok_t&)
{
    fLogmeI("ok_t{{}}");
}

// testing: no
//
void handle_server(const ok_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const ok_t&, const any_state_t&)
{
}

}// namespace uuid_protocol::v2::system

namespace uuid_protocol::v3::system
{

// testing: no
//
any_state_t create(std::type_identity<bye_t>, const any_state_t&)
{
    return bye_t{};
}

// testing: no
//
void log(const bye_t&)
{
    fLogmeI("bye_t{{}}");
}

// testing: no
//
void handle_server(const bye_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const bye_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<error_t>, const any_state_t&)
{
    return error_t{};
}

// testing: no
//
void log(const error_t&)
{
    fLogmeI("error_t{{}}");
}

// testing: no
//
void handle_server(const error_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const error_t&, const any_state_t&)
{
}

// the client hello names this version
//
// testing: no
//
any_state_t create(std::type_identity<hello_t>, const any_state_t&)
{
    return hello_t{ .message = "hello client version v3!" };
}

// testing: no
//
void log(const hello_t& value)
{
    fLogmeI("hello_t{{{}}}", value.message);
}

// testing: no
//
void handle_server(const hello_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const hello_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<ok_t>, const any_state_t&)
{
    return ok_t{};
}

// testing: no
//
void log(const ok_t&)
{
    fLogmeI("ok_t{{}}");
}

// testing: no
//
void handle_server(const ok_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const ok_t&, const any_state_t&)
{
}

}// namespace uuid_protocol::v3::system

namespace uuid_protocol::v3::payload
{

// testing: no
//
any_state_t create(std::type_identity<lamp_t>, const any_state_t&)
{
    return lamp_t{ .lit = true };
}

// testing: no
//
void log(const lamp_t& value)
{
    fLogmeI("lamp_t{{ lit: {} }}", value.lit);
}

// testing: no
//
void handle_server(const lamp_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const lamp_t&, const any_state_t&)
{
}

}// namespace uuid_protocol::v3::payload

namespace uuid_protocol
{

// the shared endpoint calls this. the generated ones are v1::registrate(), v2::registrate(), and v3::registrate()
//
// testing: no
//
void registrate()
{
    v1::registrate();
    v2::registrate();
    v3::registrate();
}

}// namespace uuid_protocol
