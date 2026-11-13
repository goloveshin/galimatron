#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/vertex_shift.hpp"


// aggregates by name, including the root
namespace galimatron::translator::state::holder::aggregate_container
{

namespace detail
{

struct record_t
{
    name_t name;
    vertex_descriptor_t vd;
    bool is_root;
    path_ot chunk_path;
    vertex_descriptor_t vd_chunk = vertex_descriptor_not_present;
    // empty for the root aggregate
    index_ot index;
    vertex_descriptor_ot vd_declarations;
    vertex_descriptor_ot vd_definitions;
    // smallest live declaration index. empty when every declaration is deleted
    index_ot first_live;
    // highest declaration index whose hash is set. empty when no declaration stores a hash
    index_ot last_hashed;
};

struct by_name
{
    using result_type = const name_t&;
    result_type operator()(const record_t& record) const
    {
        return record.name;
    }
};

struct by_vd
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd;
    }
};

struct by_index
{
    using result_type = const index_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.index;
    }
};

struct by_chunk
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_chunk;
    }
};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        random_access<>,
        ordered_non_unique
        <
            tag<by_name>,
            by_name
        >,
        ordered_unique
        <
            tag<by_vd>,
            by_vd
        >,
        ordered_non_unique
        <
            tag<by_index>,
            by_index
        >,
        ordered_non_unique
        <
            tag<by_chunk>,
            by_chunk
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
public:
    void add(
        const name_t& name,
        const vertex_descriptor_t vd,
        const bool is_root,
        const path_ot& chunk_path,
        const vertex_descriptor_t vd_chunk,
        const index_ot index
    )
    {
        push_back(detail::record_t{
            .name = name,
            .vd = vd,
            .is_root = is_root,
            .chunk_path = chunk_path,
            .vd_chunk = vd_chunk,
            .index = index,
            .vd_declarations = {},
            .vd_definitions = {},
            .first_live = {},
            .last_hashed = {}
        });
        if (is_root && !root_)
        {
            root_ = vd;
        }
    }

    // the index a new non-root aggregate receives
    //
    index_t next_index() const
    {
        const auto& index = this->template get<detail::by_index>();
        if (index.empty())
        {
            return 0;
        }
        const auto& last = *index.rbegin();
        if (!last.index)
        {
            return 0;
        }
        return *last.index + 1;
    }

    const detail::record_t* try_by_vd(const vertex_descriptor_t vd) const
    {
        const auto found = try_find_by<detail::by_vd>(vd);
        if (!found)
        {
            return nullptr;
        }
        return &found->get();
    }

    // the declarations list of this aggregate,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::holder
    //
    void note_declarations(const vertex_descriptor_t vd_aggregate, const vertex_descriptor_t vd_declarations)
    {
        note(vd_aggregate, [&](detail::record_t& record)
        {
            record.vd_declarations = vd_declarations;
        });
    }

    // the definitions list of this aggregate,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::holder
    //
    void note_definitions(const vertex_descriptor_t vd_aggregate, const vertex_descriptor_t vd_definitions)
    {
        note(vd_aggregate, [&](detail::record_t& record)
        {
            record.vd_definitions = vd_definitions;
        });
    }

    // the root aggregate is already present
    //
    bool has_root() const
    {
        return root_.has_value();
    }

    const detail::record_t& root() const
    {
        if (root_)
        {
            if (const auto* record = try_by_vd(*root_))
            {
                return *record;
            }
        }

        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::holder{},
            exception::e_what_info{ "cannot find root aggregate" }
        );
    }

    // another aggregate still lives in this chunk. the aggregate being removed does not count
    //
    bool keeps_other(const vertex_descriptor_t vd_chunk, const vertex_descriptor_t vd_aggregate) const
    {
        for (const auto& record : equal_range_by<detail::by_chunk>(vd_chunk))
        {
            if (record.vd != vd_aggregate)
            {
                return true;
            }
        }
        return false;
    }

    template <typename ... Args>
    decltype(auto) try_find_range_by_name(Args&& ... args) const
    {
        return equal_range_by<detail::by_name>(std::forward<Args>(args)...);
    }

    const detail::record_t* try_by_name(const name_t& name) const
    {
        for (const auto& record : try_find_range_by_name(name))
        {
            return &record;
        }
        return nullptr;
    }

    // the aggregate leaves the index. the caller detaches its chunk
    //
    // testing: no
    //
    void forget(const vertex_descriptor_t vd)
    {
        const bool was_root = root_ && *root_ == vd;
        try_erase_by<detail::by_vd>(vd);
        if (!was_root)
        {
            return;
        }

        root_.reset();
        for (const auto& record : *this)
        {
            if (record.is_root)
            {
                root_ = record.vd;
                return;
            }
        }
    }

    // smallest live declaration of this aggregate. empty clears it
    //
    void note_first_live(const vertex_descriptor_t vd_aggregate, const index_ot first_live)
    {
        note(vd_aggregate, [&](detail::record_t& record)
        {
            record.first_live = first_live;
        });
    }

    void note_last_hashed(const vertex_descriptor_t vd_aggregate, const index_ot last_hashed)
    {
        note(vd_aggregate, [&](detail::record_t& record)
        {
            record.last_hashed = last_hashed;
        });
    }

    // descriptors this index still names
    //
    // testing: no
    //
    void collect_vertex_descriptors(std::vector<vertex_descriptor_t>& held) const
    {
        for (const auto& record : *this)
        {
            hold_vertex(held, record.vd);
            hold_vertex(held, record.vd_chunk);
            hold_vertex(held, record.vd_declarations);
            hold_vertex(held, record.vd_definitions);
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
            pull_down(record.vd, removed);
            pull_down(record.vd_chunk, removed);
            pull_down(record.vd_declarations, removed);
            pull_down(record.vd_definitions, removed);
        });
        pull_down(root_, removed);
    }

private:
    vertex_descriptor_ot root_;

    template <typename Assign>
    void note(const vertex_descriptor_t vd_aggregate, Assign assign)
    {
        const auto result = try_find_by_and_modify<detail::by_vd>(vd_aggregate, assign);
        if (result != modify_result_et::modified)
        {
            leaf::throw_exception(
                exception::domain::translator_library{},
                exception::subdomain::translator_library::holder{},
                exception::e_what_info{ "cannot find aggregate" }
            );
        }
    }
};

}// namespace galimatron::translator::state::holder::aggregate_container
