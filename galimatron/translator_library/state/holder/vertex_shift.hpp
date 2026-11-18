#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"


namespace galimatron::translator::state::holder
{

// the vertices that leave the graph in one pass. each removed vertex leaves a hole,
// and a descriptor above it moves down by the number of holes below it.
// a descriptor that named a removed vertex becomes absent
//
class vertex_shift_t
{
public:
    // the removed descriptors, in any order. a repeated descriptor counts once
    //
    explicit vertex_shift_t(vertex_descriptors_vector_t removed)
        : removed_(std::move(removed))
    {
        removed_ |= ranges::actions::sort | ranges::actions::unique;
    }

    bool is_empty() const
    {
        return removed_.empty();
    }

    // the descriptor after the shift
    //
    vertex_descriptor_t operator()(const vertex_descriptor_t vd) const
    {
        if (vd == vertex_descriptor_not_present)
        {
            return vd;
        }
        const auto first_not_below = ranges::lower_bound(removed_, vd);
        if (first_not_below != removed_.end() && *first_not_below == vd)
        {
            return vertex_descriptor_not_present;
        }
        return vd - static_cast<vertex_descriptor_t>(first_not_below - removed_.begin());
    }

private:
    vertex_descriptors_vector_t removed_;
};

// the shift applied to a descriptor.
// a descriptor that named a removed vertex becomes absent
//
inline void pull_down(vertex_descriptor_t& vd, const vertex_shift_t& shift)
{
    vd = shift(vd);
}

// same shift for an optional descriptor. an empty optional stays empty.
// a descriptor that named a removed vertex becomes empty
//
inline void pull_down(vertex_descriptor_ot& vd, const vertex_shift_t& shift)
{
    if (!vd)
    {
        return;
    }
    pull_down(*vd, shift);
    if (*vd == vertex_descriptor_not_present)
    {
        vd.reset();
    }
}

// remembers a descriptor that still names a live vertex
//
inline void hold_vertex(vertex_descriptors_vector_t& held, const vertex_descriptor_t vd)
{
    if (vd != vertex_descriptor_not_present)
    {
        held.push_back(vd);
    }
}

// remembers an optional descriptor that still names a live vertex
//
inline void hold_vertex(vertex_descriptors_vector_t& held, const vertex_descriptor_ot& vd)
{
    if (vd)
    {
        hold_vertex(held, *vd);
    }
}

// assign shifts the descriptors of one record, and the container reindexes it.
// throws translator_library::holder when two records would share a key
//
template <typename Container, typename Assign>
void rewrite_descriptors(Container& container, Assign assign)
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
