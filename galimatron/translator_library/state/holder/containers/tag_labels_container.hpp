#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/forward.hpp"


// labels that the tag refers to
namespace galimatron::translator::state::holder::tag_labels_container
{

namespace detail
{

struct record_t
{
    // tag
    vertex_descriptor_t vd_tag;

    // its labels
    vertex_descriptors_ot vd_labels;
    aggregated_labels_static_ot aggregated_labels_static;
};

struct by_tag
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_tag;
    }
};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_unique<
            tag<by_tag>,
            by_tag
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    template <typename ... Args>
    decltype(auto) find_by_tag_and_do(Args&& ... args)
    {
        return find_by_and_do<detail::by_tag>(std::forward<Args>(args)...);
    }
};

}// namespace galimatron::translator::state::holder::tag_labels_container
