// galimatron
#include "galimatron/runtime_library/filesystem_decorator/detail/place_container.hpp"
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>

namespace galimatron::executable::filesystem_decorator::place_container
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_fsd);
}

namespace
{

// the directory a place lists, without a trailing separator
//
path_t place_directory(path_t path)
{
    path = path.lexically_normal();
    if (path.has_relative_path() && !path.has_filename())
    {
        path = path.parent_path();
    }
    return path;
}

// true when path is a descendant directory of root
//
bool is_inside(const path_t& path, const path_t& root)
{
    const auto relative = place_directory(path).lexically_relative(place_directory(root));
    if (relative.empty())
    {
        return false;
    }
    const auto first = *relative.begin();
    return first != path_t("..") && first != path_t(".");
}
}// namespace


place_id_t container_t::add_place(const place_t& args)
{
    // a place lists files under its root recursively, so one root cannot sit inside another
    for (const auto& place : *this)
    {
        const auto added_inside = is_inside(args.root, place.root);
        const auto existing_inside = is_inside(place.root, args.root);
        if (added_inside || existing_inside)
        {
            const auto& inner = added_inside ? args.root : place.root;
            const auto& outer = added_inside ? place.root : args.root;
            leaf::throw_exception(
                exception::domain::runtime_library{},
                exception::subdomain::runtime_library::filesystem_decorator{},
                exception::e_what_info{ std::format(
                    "place root '{}' is nested in '{}'",
                    place_directory(inner).generic_string(),
                    place_directory(outer).generic_string()
                ) }
            );
        }
    }

    const auto id = make_next_place_id();

    if (const auto& [ _, is_inserted ] = insert(record_t{{args}, id });
        !is_inserted)
    {
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem_decorator{},
            exception::e_what_info{ std::format("cannot add place '{}'", args.name) }
        );
    }

    return id;
}

const place_t& container_t::get_place(const place_id_t& place_id)
{
    return find_by_place(place_id);
}

container_t::record_t& container_t::find_by_place(const place_id_t& place_id)
{
    return to_non_const(find_by_or_throw<detail::by_place>(
        place_id,
        [&]
        {
            leaf::throw_exception(
                exception::domain::runtime_library{},
                exception::subdomain::runtime_library::filesystem_decorator{},
                exception::e_what_info{ std::format("cannot find place by id {}", place_id) }
            );
        }
    ));
}

container_t::record_t& container_t::find_by_alias(const place_t::string_alias_t& alias)
{
    const auto found = equal_range_by<detail::by_alias>(alias);
    const auto count = ranges::distance(found);
    if (count == 0)
    {
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem_decorator{},
            exception::e_what_info{ std::format("cannot find place by alias '{}'", alias) }
        );
    }
    if (count > 1)
    {
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem_decorator{},
            exception::e_what_info{ std::format("place alias '{}' names more than one place", alias) }
        );
    }
    return to_non_const(*found.begin());
}

detail::record_ot container_t::try_find_by_path(const path_t& path)
{
    std::map<std::size_t, const record_t*> root_size_to_place;
    {
        for (const auto& place : *this)
        {
            if (make_runtime_filesystem().has_same_root_subpath({
                .path = path,
                .path_root = place.root
            }))
            {
                root_size_to_place[place.root.string().size()] = &place;
            }
        }
    }
    if (!root_size_to_place.empty())
    {
        return *root_size_to_place.rbegin()->second;
    }
    return {};
}

place_id_t container_t::make_next_place_id()
{
    return next_place_++;
}

}// namespace galimatron::executable::filesystem_decorator::place_container
