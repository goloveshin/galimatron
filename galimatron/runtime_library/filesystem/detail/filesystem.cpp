// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::executable::filesystem::physically
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_fs);
}


namespace detail
{

const auto filesystem = gf::physically::make_filesystem();

}// namespace detail

gf::abstract::filesystem_t make_filesystem(const gf::abstract::content_filter_ft& content_filter)
{
    gf::abstract::filesystem_t filesystem
    {
        .exist = [](const auto& path)
        {
            return detail::filesystem.exist(path);
        },
        .is_regular_file = [](const auto& path)
        {
            return detail::filesystem.is_regular_file(path);
        },
        .get_recursive_directory_range = [](const auto& path)
        {
            return detail::filesystem.get_recursive_directory_range(path);
        },
        .try_read = [](const auto& path)
        {
            return detail::filesystem.try_read(
            {
                .path = path,
                .logger = [](const auto& path)
                {
                    fLogmeD("try read file '{}'", path.string());
                }
            });
        },
        .read = [](const auto& path)
        {
            return detail::filesystem.read(
            {
                .path = path,
                .logger = [](const auto& path)
                {
                    fLogmeD("read file '{}'", path.string());
                },
                .thrower = [](const auto& path)
                {
                    leaf::throw_exception(
                        exception::domain::runtime_library{},
                        exception::subdomain::runtime_library::filesystem{},
                        exception::details::io_operation_et::READ,
                        exception::e_path_info_for(path)
                    );
                }
            });
        },
        .write = [](const auto& path, const auto& file_content)
        {
            detail::filesystem.write(
            {
                .path = path,
                .content = file_content,
                .logger = [](const auto& path)
                {
                    fLogmeD("write file '{}'", path.string());
                }
            });
        },
        .remove = [](const auto& path)
        {
            detail::filesystem.remove(
            {
                .path = path,
                .logger = [](const auto& path)
                {
                    fLogmeD("remove '{}'", path.string());
                }
            });
        },
        .create_directory = [](const auto& path)
        {
            detail::filesystem.create_directory(
            {
                .path = path,
                .logger = [](const auto& path)
                {
                    fLogmeD("create directory '{}'", path.string());
                }
            });
        },
        .to_absolute = [](const auto& path)
        {
            return detail::filesystem.to_absolute(path);
        },
        .has_same_root_subpath = [](const auto& args)
        {
            return detail::filesystem.has_same_root_subpath(
            {
                .path = args.path,
                .path_root = args.path_root
            });
        },
        .get_relative_path_without_filename = [](const auto& args)
        {
            return detail::filesystem.get_relative_path_without_filename(
            {
                .path = args.path,
                .path_root = args.path_root
            });
        },
        .get_relative_path_with_filename = [](const auto& args)
        {
            return detail::filesystem.get_relative_path_with_filename(
            {
                .path = args.path,
                .path_root = args.path_root
            });
        },
        .split_path_by_parts = [](const auto& args)
        {
            return detail::filesystem.split_path_by_parts(
            {
                .path_root = args.path_root,
                .path = args.path
            });
        },
        .merge_paths = [](const auto& args)
        {
            // a missing root or file name. the path is not assembled yet
            //
            const auto throw_merge = [](const std::string& what)
            {
                leaf::throw_exception(
                    exception::domain::runtime_library{},
                    exception::subdomain::runtime_library::filesystem{},
                    exception::e_what_info{ what }
                );
            };
            return detail::filesystem.merge_paths(
            {
                .old_parts = args.old_parts,
                .new_parts = args.new_parts,
                .point_1 = throw_merge,
                .point_2 = throw_merge
            });
        },
        .compare_directories = [](const auto& args)
        {
            return detail::filesystem.compare_directories({
                .path_1 = args.path_1,
                .path_2 = args.path_2
            });
        },
        .list_files = [](const auto& args) -> std::vector<path_t>
        {
            const gf::abstract::filesystem_t filesystem{
                .exist = detail::filesystem.exist,
                .is_regular_file = detail::filesystem.is_regular_file,
                .get_recursive_directory_range = detail::filesystem.get_recursive_directory_range
            };
            auto paths =
                ranges::views::single(args.root)
                | gf::physically::views::glob_recurse_files({
                    .filesystem = filesystem,
                    .extentions = args.extentions
                })
                | ranges::to<std::vector>();
            if (args.sort)
            {
                paths |= ranges::actions::sort;
            }
            return paths;
        }
    };

    filesystem.content_filter = content_filter;
    gf::abstract::apply_content_filter(filesystem);
    gf::abstract::assign_list_files_with_deferred_content(filesystem);

    return filesystem;
}

}// namespace galimatron::executable::filesystem::physically
