#pragma once

// uuid_protocol
#include "uuid_protocol/common/message_container.hpp"
#include "uuid_protocol/common/protocol_forward.hpp"

// alpaca
#include <alpaca/alpaca.h>


namespace uuid_protocol
{

// registers one generated struct. create() is the value the endpoint sends
//
// testing: no
//
template <typename Obj>
struct registrate_t
{
    explicit registrate_t(
        const version_t version,
        const group_t group
    )
    {
        messages().registrate(
        {
            .uuid = get_uuid(std::type_identity<Obj>{}),
            .version = version,
            .group = group,
            .handle_client = [](const any_message_t& message, const any_state_t& state)
            {
                handle_client(std::any_cast<const Obj&>(message), state);
            },
            .handle_server = [](const any_message_t& message, const any_state_t& state)
            {
                handle_server(std::any_cast<const Obj&>(message), state);
            },
            .log = [](const any_message_t& message)
            {
                log(std::any_cast<const Obj&>(message));
            },
            .write_body = [](const any_message_t& message)
            {
                buffer_t bytes;
                alpaca::serialize(std::any_cast<const Obj&>(message), bytes);
                return bytes;
            },
            .read_body = [](const buffer_t& bytes) -> any_message_t
            {
                if (bytes.empty())
                {
                    return Obj{};
                }

                std::error_code ec;
                auto obj = alpaca::deserialize<Obj>(bytes, ec);
                if (ec)
                {
                    throw std::runtime_error(
                        std::format(
                            "deserialize: {}",
                            ec.message()
                        )
                    );
                }
                return obj;
            },
            .create = [](const any_state_t& state) -> any_message_t
            {
                return create(std::type_identity<Obj>{}, state);
            }
        });
    }
};

}// namespace uuid_protocol
