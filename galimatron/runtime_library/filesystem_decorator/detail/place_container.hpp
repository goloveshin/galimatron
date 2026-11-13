#pragma once

// galimatron
#include "galimatron/runtime_library/filesystem_decorator/detail/forward.hpp"


namespace galimatron::executable::filesystem_decorator::place_container
{

namespace detail
{

// what is known about a place file: content and the write and remove flags
//
struct file_t
{
    // last known content: a read or a printout
    file_content_ot content;
    // content has not been written to disk yet
    bool pending_write = false;
    // printout has already accepted this path
    bool saved = false;
    // the file was on disk at the start and was not rewritten
    bool remove_at_end = false;
};

struct record_t : place_t
{
    place_id_t id;
    std::map<path_t, file_t> files;
};
using record_ot = std::optional<std::reference_wrapper<const record_t>>;

struct by_place
{
    using result_type = place_id_t;
    result_type operator()(const record_t& record) const
    {
        return record.id;
    }
};

struct by_alias
{
    using result_type = const record_t::place_t::string_alias_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.string_alias;
    }
};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_unique
        <
            tag<by_place>,
            by_place
        >,
        ordered_non_unique
        <
            tag<by_alias>,
            by_alias
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    place_id_t add_place(const place_t&);
    const place_t& get_place(const place_id_t&);

    record_t& find_by_place(const place_id_t&);

    // the alias names exactly one place, otherwise exception::domain::runtime_library, exception::subdomain::runtime_library::filesystem_decorator
    //
    record_t& find_by_alias(const place_t::string_alias_t&);
    detail::record_ot try_find_by_path(const path_t&);

    template <typename ModifyFunc>
    void find_and_modify(const place_id_t& place_id, ModifyFunc&& modify_func)
    {
        find_by_and_modify<detail::by_place>(
            place_id,
            // on_absent
            [&]
            {
                leaf::throw_exception(
                    exception::domain::runtime_library{},
                    exception::subdomain::runtime_library::filesystem_decorator{},
                    exception::e_what_info{ std::format("cannot find place {}", place_id) }
                );
            },
            // on_rejected
            [&]
            {
                leaf::throw_exception(
                    exception::domain::runtime_library{},
                    exception::subdomain::runtime_library::filesystem_decorator{},
                    exception::e_what_info{ std::format("cannot modify place {}", place_id) }
                );
            },
            std::forward<ModifyFunc>(modify_func)
        );
    }

private:
    place_id_t make_next_place_id();

private:
    place_id_t next_place_ = 0;
};

}// namespace galimatron::executable::filesystem_decorator::place_container
