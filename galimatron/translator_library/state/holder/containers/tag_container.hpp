#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/graph/graph.hpp"


namespace galimatron::translator::state::holder::tag_container
{

namespace detail
{

struct record_t
{
    vertex_descriptor_t vd_version;
    vertex_descriptor_t vd_tag;
    name_t name;
    std::string body;
};

struct by_vd_version
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_version;
    }
};

struct by_name
{
    using result_type = const name_t&;
    result_type operator()(const record_t& record) const
    {
        return record.name;
    }
};

struct by_vd_version_and_name
{};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        random_access<>,// deque-like
        ordered_non_unique
        <
            tag<by_vd_version>,
            by_vd_version
        >,
        ordered_non_unique
        <
            tag<by_vd_version_and_name>,
            composite_key<
                record_t,
                by_vd_version,
                by_name
            >
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    template <typename ... Args>
    decltype(auto) try_find_range_by_vd_version(Args&& ... args) const
    {
        return equal_range_by<detail::by_vd_version>(std::forward<Args>(args)...);
    }

    // the compiled version already holds a tag of this name
    //
    bool contains(
        const vertex_descriptor_t vd_version,
        const name_t& name
    ) const
    {
        return try_find_by<detail::by_vd_version_and_name>(boost::make_tuple(vd_version, name)).has_value();
    }
};

}// namespace galimatron::translator::state::holder::tag_container
