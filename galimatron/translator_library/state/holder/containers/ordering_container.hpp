#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/forward.hpp"


namespace galimatron::translator::state::holder::ordering_container
{

namespace detail
{

struct record_t
{
    index_t version_index;
    const ordering_statement_t& ordering;
};

struct by_version_index
{
    using result_type = const index_t&;
    result_type operator()(const record_t& record) const
    {
        return record.version_index;
    }
};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        random_access<>,// deque-like
        ordered_non_unique<
            tag<by_version_index>,
            by_version_index
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    template <typename ... Args>
    decltype(auto) try_find_range_by_version_index(Args&& ... args) const
    {
        return equal_range_by<detail::by_version_index>(std::forward<Args>(args)...);
    }
};

}// namespace galimatron::translator::state::holder::ordering_container
