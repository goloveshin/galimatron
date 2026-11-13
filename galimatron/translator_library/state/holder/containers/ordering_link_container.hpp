#pragma once

// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/vertex_shift.hpp"


// an ordering statement names aggregates. the name compare ignores case
namespace galimatron::translator::state::holder::ordering_link_container
{

namespace detail
{

struct record_t
{
    name_t aggregate_name;
    index_ot version_index;
    vertex_descriptor_t vd_ordering = vertex_descriptor_not_present;
    name_t ordering_name;
};

struct by_aggregate_name
{
    using result_type = const name_t&;
    result_type operator()(const record_t& record) const
    {
        return record.aggregate_name;
    }
};

struct by_ordering
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_ordering;
    }
};

struct name_iless_t
{
    bool operator()(const name_t& left, const name_t& right) const
    {
        return boost::ilexicographical_compare(left, right);
    }
};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_non_unique
        <
            tag<by_aggregate_name>,
            by_aggregate_name,
            name_iless_t
        >,
        ordered_non_unique
        <
            tag<by_ordering>,
            by_ordering
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    // one row per aggregate named by this ordering
    //
    void add(
        const vertex_descriptor_t vd_ordering,
        const index_ot version_index,
        const name_t& ordering_name,
        const ordering_statement_t::ordering_parts_t& parts
    )
    {
        for (const auto& part : parts)
        {
            for (const auto& aggregate_name : part)
            {
                insert(detail::record_t{
                    .aggregate_name = aggregate_name,
                    .version_index = version_index,
                    .vd_ordering = vd_ordering,
                    .ordering_name = ordering_name
                });
            }
        }
    }

    // the ordering vertex leaves. rows that named its aggregates leave with it
    //
    void forget(const vertex_descriptor_t vd_ordering)
    {
        auto& index = this->template get<detail::by_ordering>();
        const auto range = index.equal_range(vd_ordering);
        index.erase(range.first, range.second);
    }

    // a root ordering names this aggregate, compared without case
    //
    bool names(const name_t& name) const
    {
        return !equal_range_by<detail::by_aggregate_name>(name).empty();
    }

    // descriptors this index still names
    //
    // testing: no
    //
    void collect_vertex_descriptors(std::vector<vertex_descriptor_t>& held) const
    {
        for (const auto& record : *this)
        {
            hold_vertex(held, record.vd_ordering);
        }
    }

    // descriptors above a removed vertex move down by one
    //
    // testing: no
    //
    void note_vertex_removed(const vertex_descriptor_t removed)
    {
        rewrite_descriptors(*this, removed, [&](detail::record_t& record)
        {
            pull_down(record.vd_ordering, removed);
        });
    }
};

}// namespace galimatron::translator::state::holder::ordering_link_container
