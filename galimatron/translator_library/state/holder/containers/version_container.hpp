#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/vertex_shift.hpp"


// aggregate version: declaration and definition by number
namespace galimatron::translator::state::holder::version_container
{

namespace detail
{

struct record_t
{
    vertex_descriptor_t vd_aggregate;
    index_t version_index;
    vertex_descriptor_ot vd_declaration;
    vertex_descriptor_ot vd_definition;
    version_status_oet status;
    string_ot hash;
    string_ot comment;
    std::string definition_payload;
    index_ot declaration_order;
    index_ot definition_order;
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
    using result_type = const index_t&;
    result_type operator()(const record_t& record) const
    {
        return record.version_index;
    }
};

struct by_aggregate_and_version_index
{};

struct by_declaration_order
{
    using result_type = const index_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.declaration_order;
    }
};

struct by_definition_order
{
    using result_type = const index_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.definition_order;
    }
};

struct by_aggregate_and_declaration_order
{};

struct by_aggregate_and_definition_order
{};

using namespace boost::multi_index;

using container_t = multi_index_container
<
    record_t,
    indexed_by
    <
        ordered_unique
        <
            tag<by_aggregate_and_version_index>,
            composite_key<
                record_t,
                by_aggregate,
                by_version_index
            >
        >,
        ordered_non_unique
        <
            tag<by_aggregate_and_declaration_order>,
            composite_key<
                record_t,
                by_aggregate,
                by_declaration_order
            >
        >,
        ordered_non_unique
        <
            tag<by_aggregate_and_definition_order>,
            composite_key<
                record_t,
                by_aggregate,
                by_definition_order
            >
        >
    >
>;

// an anonymous version body. it has no declaration index. owner is the version named by version before N
//
struct anonymous_record_t
{
    vertex_descriptor_t vd_aggregate = vertex_descriptor_not_present;
    vertex_descriptor_t vd_definition = vertex_descriptor_not_present;
    index_ot owner;
    index_t definition_order = 0;
};

struct anonymous_by_aggregate
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const anonymous_record_t& row) const
    {
        return row.vd_aggregate;
    }
};

struct anonymous_by_definition
{
    using result_type = const vertex_descriptor_t&;
    result_type operator()(const anonymous_record_t& row) const
    {
        return row.vd_definition;
    }
};

struct anonymous_by_order
{
    using result_type = const index_t&;
    result_type operator()(const anonymous_record_t& row) const
    {
        return row.definition_order;
    }
};

struct anonymous_by_owner
{
    using result_type = const index_ot&;
    result_type operator()(const anonymous_record_t& row) const
    {
        return row.owner;
    }
};

struct by_anonymous_definition
{};

struct by_anonymous_aggregate_and_order
{};

struct by_anonymous_aggregate_owner_and_order
{};

using anonymous_container_t = multi_index_container
<
    anonymous_record_t,
    indexed_by
    <
        ordered_unique
        <
            tag<by_anonymous_definition>,
            anonymous_by_definition
        >,
        ordered_non_unique
        <
            tag<by_anonymous_aggregate_and_order>,
            composite_key
            <
                anonymous_record_t,
                anonymous_by_aggregate,
                anonymous_by_order
            >
        >,
        ordered_non_unique
        <
            tag<by_anonymous_aggregate_owner_and_order>,
            composite_key
            <
                anonymous_record_t,
                anonymous_by_aggregate,
                anonymous_by_owner,
                anonymous_by_order
            >
        >
    >
>;

}// namespace detail

class container_t : public multi_index_lookup::container_extended_t<detail::container_t>
{
private:
    template <typename ... Args>
    decltype(auto) try_find_range_by_aggregate(Args&& ... args) const
    {
        return equal_range_by<detail::by_aggregate_and_version_index>(
            boost::make_tuple(std::forward<Args>(args)...)
        );
    }

    // declarations of this aggregate that have an order, in that order
    //
    decltype(auto) declarations_ordered(const vertex_descriptor_t vd_aggregate) const
    {
        return range_between<detail::by_aggregate_and_declaration_order>(
            boost::make_tuple(vd_aggregate, index_ot{ index_t{ 0 } }),
            boost::make_tuple(vd_aggregate, index_ot{ std::numeric_limits<index_t>::max() })
        );
    }

    // definitions of this aggregate that have an order, in that order
    //
    decltype(auto) definitions_ordered(const vertex_descriptor_t vd_aggregate) const
    {
        return range_between<detail::by_aggregate_and_definition_order>(
            boost::make_tuple(vd_aggregate, index_ot{ index_t{ 0 } }),
            boost::make_tuple(vd_aggregate, index_ot{ std::numeric_limits<index_t>::max() })
        );
    }

public:
    // a declaration was added, edited, or dropped. the holder refreshes the live and hashed indexes
    //
    void on_declaration_changed(std::function<void (const vertex_descriptor_t)> notify)
    {
        on_declaration_changed_ = std::move(notify);
    }

    void add_declaration(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index,
        const vertex_descriptor_t vd_declaration,
        const version_status_oet status,
        const string_ot& hash,
        const string_ot& comment
    )
    {
        const auto key = boost::make_tuple(vd_aggregate, version_index);
        const auto order = next_declaration_order(vd_aggregate);
        const auto result = try_find_by_and_modify<detail::by_aggregate_and_version_index>(
            key,
            [&](detail::record_t& record)
            {
                record.vd_declaration = vd_declaration;
                record.status = status;
                record.hash = hash;
                record.comment = comment;
                if (!record.declaration_order)
                {
                    record.declaration_order = order;
                }
            }
        );

        if (result == modify_result_et::absent)
        {
            insert(detail::record_t{
                .vd_aggregate = vd_aggregate,
                .version_index = version_index,
                .vd_declaration = vd_declaration,
                .vd_definition = {},
                .status = status,
                .hash = hash,
                .comment = comment,
                .definition_payload = {},
                .declaration_order = order,
                .definition_order = {}
            });
        }
        note_declaration_changed(vd_aggregate);
    }

    void add_definition(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index,
        const vertex_descriptor_t vd_definition
    )
    {
        const auto key = boost::make_tuple(vd_aggregate, version_index);
        const auto order = next_definition_order(vd_aggregate);
        const auto result = try_find_by_and_modify<detail::by_aggregate_and_version_index>(
            key,
            [&](detail::record_t& record)
            {
                record.vd_definition = vd_definition;
                if (!record.definition_order)
                {
                    record.definition_order = order;
                }
            }
        );

        if (result == modify_result_et::absent)
        {
            insert(detail::record_t{
                .vd_aggregate = vd_aggregate,
                .version_index = version_index,
                .vd_declaration = {},
                .vd_definition = vd_definition,
                .status = {},
                .hash = {},
                .comment = {},
                .definition_payload = {},
                .declaration_order = {},
                .definition_order = order
            });
        }
    }

    indices_t declaration_indices(const vertex_descriptor_t vd_aggregate) const
    {
        auto rows = declarations_ordered(vd_aggregate);
        return
            rows
            | ranges::views::transform([](const detail::record_t& record)
            {
                return record.version_index;
            })
            | ranges::to<indices_t>()
            ;
    }

    struct declaration_t
    {
        index_t index;
        version_status_oet status;
    };

    std::vector<declaration_t> declarations_in_order(const vertex_descriptor_t vd_aggregate) const
    {
        auto rows = declarations_ordered(vd_aggregate);
        return
            rows
            | ranges::views::transform([](const detail::record_t& record)
            {
                return declaration_t{
                    .index = record.version_index,
                    .status = record.status
                };
            })
            | ranges::to_vector
            ;
    }

    indices_t definition_indices(const vertex_descriptor_t vd_aggregate) const
    {
        auto rows = definitions_ordered(vd_aggregate);
        return
            rows
            | ranges::views::transform([](const detail::record_t& record)
            {
                return record.version_index;
            })
            | ranges::to<indices_t>()
            ;
    }

    struct definition_t
    {
        index_t index;
        vertex_descriptor_t vd;
    };

    std::vector<definition_t> definitions_in_order(const vertex_descriptor_t vd_aggregate) const
    {
        auto rows = definitions_ordered(vd_aggregate);
        return
            rows
            | ranges::views::transform([](const detail::record_t& record)
            {
                return definition_t{
                    .index = record.version_index,
                    .vd = record.vd_definition.value_or(vertex_descriptor_not_present)
                };
            })
            | ranges::to_vector
            ;
    }

    // one version body, numbered or anonymous. anonymous leaves index empty. file order
    //
    struct body_t
    {
        index_ot index;
        index_ot owner;
        vertex_descriptor_t vd = vertex_descriptor_not_present;
    };

    std::vector<body_t> bodies_in_order(const vertex_descriptor_t vd_aggregate) const
    {
        struct row_t
        {
            index_t order = 0;
            body_t body;
        };

        auto numbered = definitions_ordered(vd_aggregate);
        auto numbered_rows =
            numbered
            | ranges::views::transform([](const detail::record_t& record)
            {
                return row_t{
                    .order = *record.definition_order,
                    .body = body_t{
                        .index = record.version_index,
                        .owner = {},
                        .vd = record.vd_definition.value_or(vertex_descriptor_not_present)
                    }
                };
            })
            | ranges::to_vector
            ;

        auto anonymous_rows = anonymous_in_order(vd_aggregate);
        auto anonymous_bodies =
            anonymous_rows
            | ranges::views::transform([](const detail::anonymous_record_t& anonymous)
            {
                return row_t{
                    .order = anonymous.definition_order,
                    .body = body_t{
                        .index = {},
                        .owner = anonymous.owner,
                        .vd = anonymous.vd_definition
                    }
                };
            })
            | ranges::to_vector
            ;

        auto rows = std::move(numbered_rows);
        rows.insert(rows.end(), anonymous_bodies.begin(), anonymous_bodies.end());
        rows |= ranges::actions::sort(ranges::less{}, &row_t::order);
        return
            rows
            | ranges::views::transform([](const row_t& row)
            {
                return row.body;
            })
            | ranges::to_vector
            ;
    }

    vertex_descriptor_ot try_declaration(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index
    ) const
    {
        const auto found = try_find_by<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index)
        );
        if (!found || !found->get().vd_declaration)
        {
            return {};
        }
        return found->get().vd_declaration;
    }

    vertex_descriptor_ot try_definition(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index
    ) const
    {
        const auto found = try_find_by<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index)
        );
        if (!found || !found->get().vd_definition)
        {
            return {};
        }
        return found->get().vd_definition;
    }

    // version status; empty if no such version exists
    //
    version_status_oet status(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index
    ) const
    {
        const auto found = try_find_by<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index)
        );
        if (!found)
        {
            return {};
        }
        return found->get().status;
    }

    struct stored_t
    {
        string_ot hash;
        string_ot comment;
        version_status_oet status;
        std::string definition_payload;
        bool has_declaration = false;
        bool has_definition = false;
    };

    std::optional<stored_t> try_stored(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index
    ) const
    {
        const auto found = try_find_by<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index)
        );
        if (!found)
        {
            return {};
        }

        const auto& record = found->get();
        return stored_t{
            .hash = record.hash,
            .comment = record.comment,
            .status = record.status,
            .definition_payload = record.definition_payload,
            .has_declaration = record.vd_declaration.has_value(),
            .has_definition = record.vd_definition.has_value()
        };
    }

    // highest declaration index of this aggregate whose hash is set
    //
    index_ot last_hashed_index(const vertex_descriptor_t vd_aggregate) const
    {
        index_ot last;
        for (const auto& record : try_find_range_by_aggregate(vd_aggregate))
        {
            if (!record.vd_declaration || !record.hash)
            {
                continue;
            }
            if (!last || record.version_index > *last)
            {
                last = record.version_index;
            }
        }
        return last;
    }

    void set_declaration_fields(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index,
        const string_ot& hash,
        const string_ot& comment,
        const version_status_oet status
    )
    {
        try_find_by_and_modify<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index),
            [&](detail::record_t& record)
            {
                record.hash = hash;
                record.comment = comment;
                record.status = status;
            }
        );
        note_declaration_changed(vd_aggregate);
    }

    void set_definition_payload(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index,
        const std::string& definition_payload
    )
    {
        try_find_by_and_modify<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index),
            [&](detail::record_t& record)
            {
                record.definition_payload = definition_payload;
            }
        );
    }

    // status of a version that is already recorded
    //
    void set_status(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index,
        const version_status_oet status
    )
    {
        try_find_by_and_modify<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index),
            [&](detail::record_t& record)
            {
                record.status = status;
            }
        );
        note_declaration_changed(vd_aggregate);
    }

    using anonymous_t = detail::anonymous_record_t;

    // owner is empty until bind_anonymous_owners or a later rebind sees the next numbered version
    //
    void add_anonymous(
        const vertex_descriptor_t vd_aggregate,
        const vertex_descriptor_t vd_definition,
        const index_ot owner = {}
    )
    {
        anonymous_.insert(anonymous_t{
            .vd_aggregate = vd_aggregate,
            .vd_definition = vd_definition,
            .owner = owner,
            .definition_order = next_definition_order(vd_aggregate)
        });
    }

    void set_anonymous_owner(
        const vertex_descriptor_t vd_definition,
        const index_t owner
    )
    {
        anonymous_.try_find_by_and_modify<detail::by_anonymous_definition>(
            vd_definition,
            [&](anonymous_t& row)
            {
                row.owner = owner;
            }
        );
    }

    // anonymous bodies of this aggregate, in body order
    //
    std::vector<anonymous_t> anonymous_in_order(const vertex_descriptor_t vd_aggregate) const
    {
        return
            anonymous_.equal_range_by<detail::by_anonymous_aggregate_and_order>(
                boost::make_tuple(vd_aggregate)
            )
            | ranges::to_vector
            ;
    }

    // anonymous bodies owned by this numbered version, in body order
    //
    std::vector<anonymous_t> anonymous_owned_by(
        const vertex_descriptor_t vd_aggregate,
        const index_t owner
    ) const
    {
        return
            anonymous_.equal_range_by<detail::by_anonymous_aggregate_owner_and_order>(
                boost::make_tuple(vd_aggregate, index_ot{ owner })
            )
            | ranges::to_vector
            ;
    }

    // definition links of this aggregate are dropped. declarations stay. anonymous bodies are dropped
    //
    void drop_definitions(const vertex_descriptor_t vd_aggregate)
    {
        indices_t indexes;
        for (const auto& record : try_find_range_by_aggregate(vd_aggregate))
        {
            indexes.push_back(record.version_index);
        }
        for (const auto index : indexes)
        {
            try_find_by_and_modify<detail::by_aggregate_and_version_index>(
                boost::make_tuple(vd_aggregate, index),
                [](detail::record_t& record)
                {
                    record.vd_definition.reset();
                    record.definition_order.reset();
                }
            );
        }
        auto& index = anonymous_.template get<detail::by_anonymous_aggregate_and_order>();
        const auto range = index.equal_range(boost::make_tuple(vd_aggregate));
        index.erase(range.first, range.second);
    }

    // drops the version record. the caller detaches its graph vertices
    //
    // testing: no
    //
    void forget(
        const vertex_descriptor_t vd_aggregate,
        const index_t version_index
    )
    {
        try_erase_by<detail::by_aggregate_and_version_index>(
            boost::make_tuple(vd_aggregate, version_index)
        );
        note_declaration_changed(vd_aggregate);
    }

    // descriptors this index still names, including anonymous bodies
    //
    // testing: no
    //
    void collect_vertex_descriptors(std::vector<vertex_descriptor_t>& held) const
    {
        for (const auto& record : *this)
        {
            hold_vertex(held, record.vd_aggregate);
            hold_vertex(held, record.vd_declaration);
            hold_vertex(held, record.vd_definition);
        }
        for (const auto& record : anonymous_)
        {
            hold_vertex(held, record.vd_aggregate);
            hold_vertex(held, record.vd_definition);
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
            pull_down(record.vd_declaration, removed);
            pull_down(record.vd_definition, removed);
        });
        rewrite_descriptors(anonymous_, removed, [&](detail::anonymous_record_t& record)
        {
            pull_down(record.vd_aggregate, removed);
            pull_down(record.vd_definition, removed);
        });
    }

private:
    class anonymous_rows_t : public multi_index_lookup::container_extended_t<detail::anonymous_container_t>
    {
    };

    anonymous_rows_t anonymous_;
    std::function<void (const vertex_descriptor_t)> on_declaration_changed_;

    void note_declaration_changed(const vertex_descriptor_t vd_aggregate)
    {
        if (on_declaration_changed_)
        {
            on_declaration_changed_(vd_aggregate);
        }
    }

    index_t next_declaration_order(const vertex_descriptor_t vd_aggregate) const
    {
        return next_order(vd_aggregate, [](const detail::record_t& record)
        {
            return record.declaration_order.has_value();
        });
    }

    index_t next_definition_order(const vertex_descriptor_t vd_aggregate) const
    {
        const auto numbered = next_order(vd_aggregate, [](const detail::record_t& record)
        {
            return record.definition_order.has_value();
        });
        const auto anonymous_rows = anonymous_.equal_range_by<detail::by_anonymous_aggregate_and_order>(
            boost::make_tuple(vd_aggregate)
        );
        return numbered + static_cast<index_t>(ranges::distance(anonymous_rows));
    }

    template <typename HasOrder>
    index_t next_order(const vertex_descriptor_t vd_aggregate, HasOrder has_order) const
    {
        index_t count = 0;

        for (const auto& record : try_find_range_by_aggregate(vd_aggregate))
        {
            if (has_order(record))
            {
                ++count;
            }
        }

        return count;
    }

};

}// namespace galimatron::translator::state::holder::version_container
