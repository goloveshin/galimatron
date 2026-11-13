#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/forward.hpp"


// the name stored for a tag
namespace galimatron::translator::state::holder::tag_name_container
{

namespace detail
{

struct record_t
{
    vertex_descriptor_t vd_tag;
    name_t name;
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
    decltype(auto) find_by_tag(Args&& ... args) const
    {
        return find_by_or_throw<detail::by_tag>(
            std::forward<Args>(args)...,
            []
            {
                leaf::throw_exception(
                    exception::domain::translator_library{},
                    exception::subdomain::translator_library::holder{},
                    exception::e_what_info{ "cannot find by tag" }
                );
            }
        );
    }

    // the stored name. empty when compilation did not name this tag
    //
    template <typename ... Args>
    name_ot try_name_by_tag(Args&& ... args) const
    {
        const auto found = try_find_by<detail::by_tag>(std::forward<Args>(args)...);
        if (!found)
        {
            return {};
        }
        return found->get().name;
    }
};

}// namespace galimatron::translator::state::holder::tag_name_container
