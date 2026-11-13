#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/forward.hpp"


namespace galimatron::translator::state::holder::ordering_rank_container
{

namespace detail
{

struct record_t
{
    name_t aggregate_name;
    index_t version_index;
    name_t ordering_name;
    index_t rank;
};

struct by_aggregate_name
{
    using result_type = const name_t&;
    result_type operator()(const record_t& record) const
    {
        return record.aggregate_name;
    }
};

struct by_version_index
{
    using result_type = const index_t&;
    result_type operator()(const record_t& record) const
    {
        return record.version_index;
    }
};

struct by_aggregate_name_and_version_index
{};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_non_unique
        <
            tag<by_aggregate_name_and_version_index>,
            composite_key<
                record_t,
                by_aggregate_name,
                by_version_index
            >
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    template <typename ... Args>
    decltype(auto) try_find_range_by_aggregate_name_and_version_index(Args&& ... args) const
    {
        return equal_range_by<detail::by_aggregate_name_and_version_index>(std::forward<Args>(args)...);
    }
};

}// namespace galimatron::translator::state::holder::ordering_rank_container
