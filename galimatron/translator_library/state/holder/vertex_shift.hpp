#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"


namespace galimatron::translator::state::holder
{

// a removed vertex leaves a hole that the next descriptors close.
// a descriptor that named the removed vertex becomes absent
//
inline void pull_down(vertex_descriptor_t& vd, const vertex_descriptor_t removed)
{
    if (vd == vertex_descriptor_not_present || vd < removed)
    {
        return;
    }
    if (vd == removed)
    {
        vd = vertex_descriptor_not_present;
        return;
    }
    --vd;
}

// same shift for an optional descriptor. an empty optional stays empty.
// a descriptor that named the removed vertex becomes empty
//
inline void pull_down(vertex_descriptor_ot& vd, const vertex_descriptor_t removed)
{
    if (!vd)
    {
        return;
    }
    pull_down(*vd, removed);
    if (*vd == vertex_descriptor_not_present)
    {
        vd.reset();
    }
}

// remembers a descriptor that still names a live vertex
//
inline void hold_vertex(std::vector<vertex_descriptor_t>& held, const vertex_descriptor_t vd)
{
    if (vd != vertex_descriptor_not_present)
    {
        held.push_back(vd);
    }
}

// remembers an optional descriptor that still names a live vertex
//
inline void hold_vertex(std::vector<vertex_descriptor_t>& held, const vertex_descriptor_ot& vd)
{
    if (vd)
    {
        hold_vertex(held, *vd);
    }
}

// every stored descriptor above the removed vertex moves down by one.
// throws translator_library::holder when two records would share a key
//
template <typename Container, typename Assign>
void rewrite_descriptors(Container& container, const vertex_descriptor_t removed, Assign assign)
{
    auto iterators =
        container
        | ranges::views::transform([&container](const auto& record)
        {
            return container.iterator_to(record);
        })
        | ranges::to_vector
        ;
    for (const auto iterator : iterators)
    {
        const auto modified = container.modify(iterator, [&](auto& record)
        {
            assign(record);
        });
        if (!modified)
        {
            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::holder{},
                exception::e_what_info{ "a vertex descriptor collided after a vertex was removed" }
            );
        }
    }
}

}// namespace galimatron::translator::state::holder
