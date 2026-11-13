#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"
#include "uuid_protocol/common/protocol_forward.hpp"

// Boost
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace uuid_protocol
{

namespace detail
{

struct by_uuid
{
    using result_type = const uuid_t&;

    // the uuid index key
    //
    // testing: no
    //
    result_type operator()(const message_operations_t& record) const
    {
        return record.uuid;
    }
};

struct by_version
{
    using result_type = const version_t&;

    // the version index key
    //
    // testing: no
    //
    result_type operator()(const message_operations_t& record) const
    {
        return record.version;
    }
};

struct by_group
{
    using result_type = const group_t&;

    // the group index key
    //
    // testing: no
    //
    result_type operator()(const message_operations_t& record) const
    {
        return record.group;
    }
};

struct by_version_and_group
{};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    message_operations_t,
    indexed_by
    <
        ordered_unique
        <
            tag<by_uuid>,
            by_uuid
        >,
        // payload messages share no_group, so a version has several of those
        //
        ordered_non_unique
        <
            tag<by_version_and_group>,
            composite_key<
                message_operations_t,
                by_version,
                by_group
            >
        >,
        ordered_non_unique
        <
            tag<by_group>,
            by_group
        >
    >
>;

}// namespace detail

// messages: one by uuid. a system group is one message
//
class message_container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    // duplicate uuid: std::logic_error
    // a second system message for the same version and group: std::logic_error
    //
    // testing: no
    //
    void registrate(const message_operations_t& operations)
    {
        if (try_find_by<detail::by_uuid>(operations.uuid))
        {
            throw std::logic_error(
                std::format(
                    "duplicate uuid of message {}",
                    boost::uuids::to_string(operations.uuid)
                )
            );
        }
        const auto same_system_message = operations.group != no_group
            && try_find_by<detail::by_version_and_group>(
                boost::make_tuple(operations.version, operations.group)
            );
        if (same_system_message)
        {
            throw std::logic_error(
                std::format(
                    "duplicate version {} and group {}",
                    operations.version,
                    operations.group
                )
            );
        }
        insert(operations);
    }

    // testing: no
    //
    std::optional<std::reference_wrapper<const message_operations_t>> try_find_by_uuid(const uuid_t& uuid) const
    {
        return try_find_by<detail::by_uuid>(uuid);
    }

    // no such message: std::logic_error
    //
    // testing: no
    //
    const message_operations_t& find_by_uuid(const uuid_t& uuid) const
    {
        return find_by_or_throw<detail::by_uuid>(uuid, [&]
        {
            throw std::logic_error(
                std::format(
                    "cannot find message by uuid {}",
                    boost::uuids::to_string(uuid)
                )
            );
        });
    }

    // no such version and group: std::logic_error
    //
    // testing: no
    //
    const message_operations_t& find_by_version_and_group(
        const version_t version,
        const group_t group
    ) const
    {
        return find_by_or_throw<detail::by_version_and_group>(
            boost::make_tuple(version, group),
            [&]
            {
                throw std::logic_error(
                    std::format(
                        "cannot find version {} and group {}",
                        version,
                        group
                    )
                );
            }
        );
    }

    // no such group: std::logic_error
    //
    // testing: no
    //
    const message_operations_t& find_by_group(const group_t group) const
    {
        return find_by_or_throw<detail::by_group>(group, [&]
        {
            throw std::logic_error(
                std::format(
                    "cannot find group {}",
                    group
                )
            );
        });
    }
};

// testing: no
//
inline message_container_t& messages()
{
    static message_container_t messages;
    return messages;
}

}// namespace uuid_protocol
