#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/forward.hpp"


// static labels
namespace galimatron::translator::state::holder::static_label_container
{

namespace detail
{

struct record_t
{
    // label
    vertex_descriptor_t vd_label;

    // its static data
    aggregated_label_static_t static_part;
};

struct by_label
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_label;
    }
};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_unique<
            tag<by_label>,
            by_label
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    template <typename ... Args>
    decltype(auto) find_by_label_and_do(Args&& ... args)
    {
        return find_by_and_do<detail::by_label>(std::forward<Args>(args)...);
    }
};

}// namespace galimatron::translator::state::holder::static_label_container
