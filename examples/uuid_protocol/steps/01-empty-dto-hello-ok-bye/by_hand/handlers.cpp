// uuid_protocol
#include "uuid_protocol/common/protocol.hpp"
#include "v1/v1.hpp"


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

namespace uuid_protocol
{

// the shared endpoint calls this. the generated one is v1::registrate()
//
// testing: no
//
void registrate()
{
    v1::registrate();
}

// this step has no payload test cases
//
// testing: no
//
void registrate_test_cases()
{
}

}// namespace uuid_protocol
