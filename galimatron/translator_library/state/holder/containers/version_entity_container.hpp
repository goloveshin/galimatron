#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"


namespace galimatron::translator::state::holder::version_entity_container
{

namespace detail
{

struct record_t
{
    name_ot name;
    entity_et entity_type;
    vertex_descriptor_t vd_entity;
    index_t version_index;
    index_t index_in_versions;// across versions
    // parked from a collapsed body. once and local stay out of the owner version
    bool from_anonymous = false;
};

struct by_name
{
    using result_type = const name_ot&;
    result_type operator()(const record_t& record) const
    {
        return record.name;
    }
};

struct by_entity_type
{
    using result_type = const entity_et&;
    result_type operator()(const record_t& record) const
    {
        return record.entity_type;
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

struct by_index_in_versions
{
    using result_type = const index_t&;
    result_type operator()(const record_t& record) const
    {
        return record.index_in_versions;
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

struct by_entity_type_and_name{};
struct by_entity_type_and_version_index{};
struct by_entity_type_and_name_and_index_in_versions{};
struct by_name_and_index_in_versions{};

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
        >,
        ordered_non_unique<
            tag<by_entity_type_and_name>,
            composite_key<
                record_t,
                by_entity_type,
                by_name
            >
        >,
        ordered_non_unique<
            tag<by_entity_type_and_version_index>,
            composite_key<
                record_t,
                by_entity_type,
                by_version_index
            >
        >,
        ordered_non_unique<
            tag<by_entity_type_and_name_and_index_in_versions>,
            composite_key<
                record_t,
                by_entity_type,
                by_name,
                by_index_in_versions
            >
        >,
        ordered_unique<
            tag<by_entity>,
            by_entity
        >,
        ordered_non_unique<
            tag<by_name_and_index_in_versions>,
            composite_key<
                record_t,
                by_name,
                by_index_in_versions
            >
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

    template <typename ... Args>
    decltype(auto) try_find_range_by_entity_type_and_name(Args&& ... args) const
    {
        return equal_range_by<detail::by_entity_type_and_name>(std::forward<Args>(args)...);
    }

    // entities of this type in this version
    //
    decltype(auto) range_by_entity_type_and_version_index(const entity_et entity_type, const index_t version_index) const
    {
        return equal_range_by<detail::by_entity_type_and_version_index>(
            boost::make_tuple(entity_type, version_index)
        );
    }

    // entities of this type with version_index from 0 up to and including version_index
    //
    decltype(auto) range_by_entity_type_up_to_version_index(const entity_et entity_type, const index_t version_index) const
    {
        return range_between<detail::by_entity_type_and_version_index>(
            boost::make_tuple(entity_type, 0),
            boost::make_tuple(entity_type, version_index)
        );
    }

    // entities of this type and name with index_in_versions from 0 up to and including index_in_versions
    //
    decltype(auto) range_by_entity_type_and_name_up_to_index_in_versions(
        const entity_et entity_type,
        const std::string& name,
        const index_t index_in_versions
    ) const
    {
        return range_between<detail::by_entity_type_and_name_and_index_in_versions>(
            boost::make_tuple(entity_type, name, 0),
            boost::make_tuple(entity_type, name, index_in_versions)
        );
    }

    // one statement. empty when this vertex was not merged
    //
    const detail::record_t* try_by_entity(const vertex_descriptor_t vd_entity) const
    {
        const auto found = try_find_by<detail::by_entity>(vd_entity);
        if (!found)
        {
            return nullptr;
        }
        return &found->get();
    }

    // the compiler resolved an unnamed tag. the name index follows the change
    //
    void note_name(const vertex_descriptor_t vd_entity, const name_t& name)
    {
        try_find_by_and_modify<detail::by_entity>(
            vd_entity,
            [&](detail::record_t& record)
            {
                record.name = name;
            }
        );
    }

    // entities of this name with index_in_versions from 0 up to and including index_in_versions.
    // tags and deletes of the same name are one sequence
    //
    decltype(auto) range_by_name_up_to_index_in_versions(
        const std::string& name,
        const index_t index_in_versions
    ) const
    {
        return range_between<detail::by_name_and_index_in_versions>(
            boost::make_tuple(name, index_t{ 0 }),
            boost::make_tuple(name, index_in_versions)
        );
    }
};

}// namespace galimatron::translator::state::holder::version_entity_container
