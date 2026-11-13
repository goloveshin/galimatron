// uuid_protocol
#include "uuid_protocol/common/protocol.hpp"
#include "uuid_protocol/common/state.hpp"
#include "v1/v1.hpp"
#include "v2/v2.hpp"
#include "v3/v3.hpp"
#include "v4/v4.hpp"


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

namespace uuid_protocol::v4::system
{

// the client bye names this version
//
// testing: no
//
any_state_t create(std::type_identity<bye_t>, const any_state_t&)
{
    return bye_t{ .message = "bye client version v4!" };
}

// testing: no
//
void log(const bye_t& value)
{
    fLogmeI("bye_t{{{}}}", value.message);
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

// the server stored the refusal with io_notify. the field carries that text
//
// testing: no
//
any_state_t create(std::type_identity<error_t>, const any_state_t& any_state)
{
    return error_t{ .message = io_message(any_state) };
}

// testing: no
//
void log(const error_t& value)
{
    fLogmeI("error_t{{{}}}", value.message);
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
    return hello_t{ .message = "hello client version v4!" };
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

}// namespace uuid_protocol::v4::system

namespace uuid_protocol::v4::payload
{

// testing: no
//
any_state_t create(std::type_identity<fog_t>, const any_state_t&)
{
    return fog_t{ .metres = 250 };
}

// testing: no
//
void log(const fog_t& value)
{
    fLogmeI("fog_t{{ metres: {} }}", value.metres);
}

// testing: no
//
void handle_server(const fog_t&, const any_state_t&)
{
}

// testing: no
//
void handle_client(const fog_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<lamp_t>, const any_state_t&)
{
    return lamp_t{ .lit = true, .candela = 1500 };
}

// testing: no
//
void log(const lamp_t& value)
{
    fLogmeI("lamp_t{{ lit: {}, candela: {} }}", value.lit, value.candela);
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

}// namespace uuid_protocol::v4::payload

namespace uuid_protocol
{

// the shared endpoint calls this. the generated ones are v1::registrate(), v2::registrate(), v3::registrate(), and v4::registrate()
//
// testing: no
//
void registrate()
{
    v1::registrate();
    v2::registrate();
    v3::registrate();
    v4::registrate();
}

}// namespace uuid_protocol
