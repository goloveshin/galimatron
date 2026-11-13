#pragma once

// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/vertex_shift.hpp"


// direct statements of one aggregate version definition
namespace galimatron::translator::state::holder::version_statement_container
{

namespace detail
{

struct record_t
{
    vertex_descriptor_t vd_aggregate;
    // empty when the statement sits in an anonymous body
    index_ot version_index;
    vertex_descriptor_t vd_definition = vertex_descriptor_not_present;
    vertex_descriptor_t vd_entity;
    name_ot name;
    index_t index_in_version = 0;
    bool names_tag = false;
    entity_et entity_type = entity_et::ROOT;
};

struct by_aggregate
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_aggregate;
    }
};

struct by_version_index
{
    using result_type = const index_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.version_index;
    }
};

struct by_definition
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_definition;
    }
};

struct by_index_in_version
{
    using result_type = const index_t&;
    result_type operator()(const record_t& record) const
    {
        return record.index_in_version;
    }
};

struct by_name
{
    using result_type = const name_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.name;
    }
};

struct by_entity
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const record_t& record) const
    {
        return record.vd_entity;
    }
};

struct by_aggregate_version_and_index
{};

struct by_aggregate_version_and_name
{};

struct by_definition_and_index
{};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_unique
        <
            tag<by_entity>,
            by_entity
        >,
        ordered_non_unique
        <
            tag<by_aggregate_version_and_index>,
            composite_key<
                record_t,
                by_aggregate,
                by_version_index,
                by_index_in_version
            >
        >,
        ordered_non_unique
        <
            tag<by_aggregate_version_and_name>,
            composite_key<
                record_t,
                by_aggregate,
                by_version_index,
                by_name
            >
        >,
        ordered_non_unique
        <
            tag<by_definition_and_index>,
            composite_key<
                record_t,
                by_definition,
                by_index_in_version
            >
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
private:
    // the statements of one aggregate version. defined here because the return type is deduced
    //
    decltype(auto) statements(const vertex_descriptor_t vd_aggregate, const index_t version_index) const
    {
        return equal_range_by<detail::by_aggregate_version_and_index>(
            boost::make_tuple(vd_aggregate, index_ot{ version_index })
        );
    }

public:
    struct statement_t
    {
        vertex_descriptor_t vd;
        entity_et entity_type;
        index_t index_in_version;
    };

    // the statements of one version, in index order
    //
    std::vector<statement_t> in_order(const vertex_descriptor_t vd_aggregate, const index_t version_index) const
    {
        return statements_from(statements(vd_aggregate, version_index));
    }

    // the statements of one version body, numbered or anonymous, in index order
    //
    std::vector<statement_t> of_definition(const vertex_descriptor_t vd_definition) const
    {
        auto range = equal_range_by<detail::by_definition_and_index>(boost::make_tuple(vd_definition));
        return statements_from(range);
    }

    // the body is one statement, and that statement is delete;
    //
    bool is_only_version_delete(const vertex_descriptor_t vd_definition) const
    {
        auto range = equal_range_by<detail::by_definition_and_index>(boost::make_tuple(vd_definition));
        auto it = range.begin();
        if (it == range.end())
        {
            return false;
        }
        const auto& record = *it;
        ++it;
        if (it != range.end())
        {
            return false;
        }
        return record.entity_type == entity_et::OPERATION_STATEMENT && !record.names_tag;
    }

private:
    template <typename Range>
    static std::vector<statement_t> statements_from(Range range)
    {
        return
            range
            | ranges::views::transform([](const detail::record_t& record)
            {
                return statement_t{
                    .vd = record.vd_entity,
                    .entity_type = record.entity_type,
                    .index_in_version = record.index_in_version
                };
            })
            | ranges::to_vector
            ;
    }

public:
    // false when this vertex is already recorded
    //
    bool add(const detail::record_t& record)
    {
        if (try_find_by<detail::by_entity>(record.vd_entity))
        {
            return false;
        }
        insert(record);
        return true;
    }

    void forget(const vertex_descriptor_t vd_entity)
    {
        auto& index = this->template get<detail::by_entity>();
        if (const auto it = index.find(vd_entity); it != index.end())
        {
            index.erase(it);
        }
    }

    bool names_tag(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index,
        const name_t& name
    ) const
    {
        for (const auto& record : equal_range_by<detail::by_aggregate_version_and_name>(
            boost::make_tuple(vd_aggregate, index_ot{ version_index }, name_ot{ name })
        ))
        {
            if (record.names_tag)
            {
                return true;
            }
        }
        return false;
    }

    index_t next_index(const vertex_descriptor_t vd_aggregate, const index_t version_index) const
    {
        const auto range = statements(vd_aggregate, version_index);
        if (range.empty())
        {
            return 0;
        }
        return std::prev(range.end())->index_in_version + 1;
    }

    // descriptors this index still names
    //
    // testing: no
    //
    void collect_vertex_descriptors(std::vector<vertex_descriptor_t>& held) const
    {
        for (const auto& record : *this)
        {
            hold_vertex(held, record.vd_aggregate);
            hold_vertex(held, record.vd_definition);
            hold_vertex(held, record.vd_entity);
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
            pull_down(record.vd_aggregate, removed);
            pull_down(record.vd_definition, removed);
            pull_down(record.vd_entity, removed);
        });
    }
};

}// namespace galimatron::translator::state::holder::version_statement_container
