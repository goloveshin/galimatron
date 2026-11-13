#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/graph/vertex.hpp"


namespace galimatron::translator::state::holder::vertex_container
{

namespace detail
{

using record_t = vertex_t;

// the index stores this, not the vertex. the vertex is mutable because Boost.Graph writes
// edge lists through the index, and that index only hands out a const element
//
template <typename StoredVertexType>
struct vertex_slot_t
{
    vertex_slot_t() = default;

    // emplace_back(property) builds the stored vertex from the property
    //
    template <typename Property>
    explicit vertex_slot_t(const Property& property)
        : vertex(property)
    {}

    mutable StoredVertexType vertex;
};

template <typename StoredVertexType>
const record_t& get_record(const StoredVertexType& stored_vertex)
{
    return stored_vertex.m_property;
}

template <typename StoredVertexType>
const record_t& get_record(const vertex_slot_t<StoredVertexType>& stored)
{
    return stored.vertex.m_property;
}

template <typename StoredVertexType>
struct by_entity_type
{
    using result_type = const entity_et&;
    result_type operator()(const vertex_slot_t<StoredVertexType>& stored) const
    {
        return get_record(stored).entity_type;
    }
};

template <typename StoredVertexType>
struct by_vd_parent
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const vertex_slot_t<StoredVertexType>& stored) const
    {
        return get_record(stored).vd_parent;
    }
};

struct by_parent_and_entity_type{};

using namespace boost::multi_index;

template <typename StoredVertexType>
using container_t = multi_index_container
<
    vertex_slot_t<StoredVertexType>,
    indexed_by
    <
        random_access<>,// deque-like
        ordered_non_unique
        <
            tag<by_entity_type<StoredVertexType>>,
            by_entity_type<StoredVertexType>
        >,
        ordered_non_unique<
            tag<by_parent_and_entity_type>,
            composite_key<
                vertex_slot_t<StoredVertexType>,
                by_vd_parent<StoredVertexType>,
                by_entity_type<StoredVertexType>
            >
        >
    >
>;

}// namespace detail


template
<
    typename StoredVertexType,
    typename nested_container_t = multi_index_lookup::container_extended_t<detail::container_t<StoredVertexType>>
>
class container_t : public nested_container_t
{
public:
    using by_entity_type = detail::by_entity_type<StoredVertexType>;
    using by_parent_and_entity_type = detail::by_parent_and_entity_type;

public:
    // Boost.Graph writes edge lists on the stored vertex.
    // a const container still returns a const vertex
    //
    StoredVertexType& operator[](const auto index)
    {
        return nested_container_t::operator[](index).vertex;
    }
    const StoredVertexType& operator[](const auto index) const
    {
        return nested_container_t::operator[](index).vertex;
    }

    // indexed fields, such as the parent, change here so the ordered indexes stay current
    //
    template <typename Modify>
    bool modify_at(const auto index, Modify&& modify_vertex)
    {
        const auto iterator = this->begin() + index;
        return this->modify(iterator, [&](auto& slot)
        {
            modify_vertex(slot.vertex);
        });
    }

public:
    template <typename ... Args>
    decltype(auto) try_find_range_by_entity_type(Args&& ... args) const
    {
        return nested_container_t::template equal_range_by<by_entity_type>(std::forward<Args>(args)...);
    }

    template <typename ... Args>
    decltype(auto) try_find_range_by_parent_and_entity_type(Args&& ... args) const
    {
        return nested_container_t::template equal_range_by<by_parent_and_entity_type>(std::forward<Args>(args)...);
    }

public:
    const StoredVertexType& find_by_entity_type(const entity_et& value) const
    {
        return nested_container_t::template find_by_or_throw<by_entity_type>(
            value,
            [&]
            {
                leaf::throw_exception(
                    exception::domain::translator_library{},
                    exception::subdomain::translator_library::holder{},
                    exception::e_what_info{ std::format("cannot find by entity type '{}'", to_string(value)) }
                );
            }
        ).vertex;
    }
};

}// namespace galimatron::translator::state::holder::vertex_container


// used often; placed directly in state for convenience
namespace galimatron::translator::state
{

namespace hvcd = holder::vertex_container::detail;

template <typename StoredVertexType>
auto get_record(const StoredVertexType& stored_vertex) -> const hvcd::record_t&
{
    return hvcd::get_record(stored_vertex);
};

template <typename Entity, typename StoredVertexType>
auto get_entity(const StoredVertexType& stored_vertex) -> const Entity&
{
    return std::get<Entity>(hvcd::get_record(stored_vertex).entity);
};

template <typename Entity, typename StoredVertexType>
auto get_record_and_entity(const StoredVertexType& stored_vertex)
{
    const auto& record = hvcd::get_record(stored_vertex);
    const auto& entity = std::get<Entity>(record.entity);
    return std::tuple<decltype(record), decltype(entity)>{ record, entity };// TODO const &
};

template <typename Entity>
auto get_entity(const hvcd::record_t& record) -> const Entity&
{
    return std::get<Entity>(record.entity);
};

template <typename Entity>
auto get_entity(hvcd::record_t& record) -> Entity&
{
    return std::get<Entity>(record.entity);
};

template <typename Entity>
auto get_record_and_entity(const hvcd::record_t& record)
{
    const auto& entity = std::get<Entity>(record.entity);
    return std::tuple<decltype(record), decltype(entity)>{ record, entity };// TODO const &
};

}// namespace galimatron::translator::state
