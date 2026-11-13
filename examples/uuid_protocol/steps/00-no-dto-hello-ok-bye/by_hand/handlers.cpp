// uuid_protocol
#include "uuid_protocol/common/protocol.hpp"
#include "uuid_protocol/common/message_container.hpp"
#include "uuid_protocol/common/protocol_forward.hpp"


namespace uuid_protocol
{

namespace
{

// one system group. the log line after the direction is the body, and there is no dto
//
// testing: no
//
void registrate_system(
    const uuid_t& uuid,
    const group_t group
)
{
    messages().registrate(
    {
        .uuid = uuid,
        .group = group,
        .handle_client = [](const any_message_t&, const any_state_t&){},
        .handle_server = [](const any_message_t&, const any_state_t&){},
        .log = [](const any_message_t&){ fLogmeI("no dto"); }
    });
}

}// namespace

// testing: no
//
void registrate()
{
    registrate_system(uuid_gen("6f1e0000-0000-4000-8000-000000000001"), hello_group);
    registrate_system(uuid_gen("6f1e0000-0000-4000-8000-000000000002"), bye_group);
    registrate_system(uuid_gen("6f1e0000-0000-4000-8000-000000000003"), ok_group);
    registrate_system(uuid_gen("6f1e0000-0000-4000-8000-000000000004"), error_group);
}

// this step has no payload test cases
//
// testing: no
//
void registrate_test_cases()
{
}

}// namespace uuid_protocol
