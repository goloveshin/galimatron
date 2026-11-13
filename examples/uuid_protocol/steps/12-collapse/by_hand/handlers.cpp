// uuid_protocol
#include "uuid_protocol/common/protocol.hpp"
#include "uuid_protocol/common/state.hpp"
#include "uuid_protocol/steps/12-collapse/by_hand/state_internal.hpp"
#include "v6/v6.hpp"


namespace uuid_protocol::v6::system
{

// the client bye names this version
//
// testing: no
//
any_state_t create(std::type_identity<bye_t>, const any_state_t&)
{
    return bye_t{ .message = "bye client version v6!" };
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
    return hello_t{ .message = "hello client version v6!" };
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

}// namespace uuid_protocol::v6::system

namespace uuid_protocol::v6::payload
{

// testing: no
//
any_state_t create(std::type_identity<watch_t>, const any_state_t&)
{
    return watch_t{
        .day = "1912-03-14",
        .lamp = { .lit = true, .candela = 1200 },
        .fog = { .metres = 200, .sea = sea_state_e::gale }
    };
}

// testing: no
//
void log(const watch_t& value)
{
    fLogmeI(
        "watch_t{{ day: '{}', lamp: {{ lit: {}, candela: {} }}, fog: {{ metres: {}, sea: {} }} }}",
        value.day,
        value.lamp.lit,
        value.lamp.candela,
        value.fog.metres,
        print(value.fog.sea)
    );
}

// testing: no
//
void handle_server(const watch_t& value, const any_state_t& any_state)
{
    cast_to_server_state(any_state).incoming(value);
}

// testing: no
//
void handle_client(const watch_t&, const any_state_t&)
{
}

// testing: no
//
any_state_t create(std::type_identity<vessel_t>, const any_state_t&)
{
    return vessel_t{
        .kind = vessel_kind_e::launch,
        .bearing = 15,
        .name = "North Light"
    };
}

// testing: no
//
void log(const vessel_t& value)
{
    fLogmeI(
        "vessel_t{{ kind: {}, bearing: {}, name: '{}' }}",
        print(value.kind),
        value.bearing,
        value.name
    );
}

// testing: no
//
void handle_server(const vessel_t& value, const any_state_t& any_state)
{
    cast_to_server_state(any_state).incoming(value);
}

// testing: no
//
void handle_client(const vessel_t&, const any_state_t&)
{
}

}// namespace uuid_protocol::v6::payload

namespace uuid_protocol
{

// the shared endpoint calls this. the generated one is v6
//
// testing: no
//
void registrate()
{
    v6::registrate();
}

}// namespace uuid_protocol
