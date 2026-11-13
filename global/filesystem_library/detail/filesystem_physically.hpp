// include only from global/filesystem_library/filesystem.hpp

// Implementation of filesystem::abstract through std::filesystem,
// which uses calls to an arbitrary logger and to exceptions
// that the user will provide
// (for example, in galimatron runtime_library does this).
//
namespace global::filesystem::physically
{

namespace detail
{

using logger_ft = std::function<void (const path_t&)>;
using thrower_ft = std::function<void (const path_t&)>;
using thrower_what_ft = std::function<void (const std::string&)>;

}// namespace detail

using exist_ft = std::function<bool (const path_t&)>;
using is_regular_file_ft = std::function<bool (const path_t&)>;
using get_recursive_directory_range_ft = std::function<ranges::any_view<fs::directory_entry> (const path_t&)>;

struct try_read_t
{
    path_t path;
    detail::logger_ft logger;
};
using try_read_ft = std::function<file_content_ot (const try_read_t&)>;

struct read_t
{
    path_t path;
    detail::logger_ft logger;
    detail::thrower_ft thrower;
};
using read_ft = std::function<file_content_t (const read_t&)>;

struct write_t
{
    path_t path;
    file_content_t content;
    detail::logger_ft logger;
};
using write_ft = std::function<void (const write_t&)>;

struct remove_t
{
    path_t path;
    detail::logger_ft logger;
};
using remove_ft = std::function<void (const remove_t&)>;

struct create_directory_t
{
    path_t path;
    detail::logger_ft logger;
};
using create_directory_ft = std::function<void (const create_directory_t&)>;

using to_absolute_ft = std::function<path_t (const path_t&)>;

struct has_same_root_subpath_t
{
    path_t path;
    path_t path_root;
};
using has_same_root_subpath_ft = std::function<bool (const has_same_root_subpath_t&)>;

struct get_relative_path_without_filename_t
{
    path_t path;
    path_t path_root;
};
using get_relative_path_without_filename_ft = std::function<path_t (const get_relative_path_without_filename_t&)>;

struct get_relative_path_with_filename_t
{
    path_t path;
    path_t path_root;
};
using get_relative_path_with_filename_ft = std::function<path_t (const get_relative_path_with_filename_t&)>;

struct split_path_by_parts_t
{
    path_t path_root;
    path_t path;
};
using split_path_by_parts_ft = std::function<path_by_parts_t (const split_path_by_parts_t&)>;

struct merge_paths_t
{
    path_by_parts_t old_parts;
    path_by_parts_t new_parts;
    detail::thrower_what_ft point_1;
    detail::thrower_what_ft point_2;
};
using merge_paths_ft = std::function<path_t (const merge_paths_t&)>;

// git diff --no-index path_1 path_2. the result carries the unified diff
//
struct compare_directories_t
{
    path_t path_1;
    path_t path_2;
};
using compare_directories_ft = std::function<directory_compare_result_t (const compare_directories_t&)>;

struct filesystem_t
{
    exist_ft exist;
    is_regular_file_ft is_regular_file;
    get_recursive_directory_range_ft get_recursive_directory_range;
    try_read_ft try_read;
    read_ft read;
    write_ft write;
    remove_ft remove;
    create_directory_ft create_directory;
    to_absolute_ft to_absolute;
    has_same_root_subpath_ft has_same_root_subpath;
    get_relative_path_without_filename_ft get_relative_path_without_filename;
    get_relative_path_with_filename_ft get_relative_path_with_filename;
    split_path_by_parts_ft split_path_by_parts;
    merge_paths_ft merge_paths;
    compare_directories_ft compare_directories;
};

filesystem_t make_filesystem();

}// namespace global::filesystem::physically

namespace global::filesystem::physically::views
{

//inline ranges::any_view<path_t> glob_recurse_files(const glob_recurse_files_t& args)
//
struct glob_recurse_files_t
{
    const abstract::filesystem_t& filesystem;
    std::set<path_t> extentions;
};
inline auto glob_recurse_files(const glob_recurse_files_t& args)
{
    namespace rv = ranges::views;

    const auto exist = args.filesystem.exist;
    const auto is_regular_file = args.filesystem.is_regular_file;
    const auto get_recursive_directory_range = args.filesystem.get_recursive_directory_range;

    return
        rv::filter([exist](const path_t& path){ return exist(path); })
        | rv::transform([get_recursive_directory_range](const path_t& path)
        {
            return get_recursive_directory_range(path);
        })
        | rv::cache1
        | rv::join
        | rv::filter([is_regular_file](const fs::directory_entry& dir_entry)
        {
            return is_regular_file(dir_entry.path());
        })
        | rv::transform([](const fs::directory_entry& dir_entry){ return dir_entry.path(); })
        | rv::filter([ext = args.extentions](const path_t& path){ return ext.contains(path.extension()); })
        ;
}

}// namespace global::filesystem::physically::views
