// include only from global/filesystem_library/filesystem.hpp

// Declaration of filesystem capabilities.
//
namespace global::filesystem::abstract
{

using exist_ft = std::function<bool (const path_t&)>;
using is_regular_file_ft = std::function<bool (const path_t&)>;
using get_recursive_directory_range_ft = std::function<ranges::any_view<fs::directory_entry> (const path_t&)>;
using try_read_ft = std::function<file_content_ot (const path_t&)>;
using read_ft = std::function<file_content_t (const path_t&)>;
using write_ft = std::function<void (const path_t&, const file_content_t&)>;
using remove_ft = std::function<void (const path_t&)>;
using create_directory_ft = std::function<void (const path_t&)>;
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

// root files with the given extensions
//
struct list_files_t
{
    path_t root;
    std::set<path_t> extentions;
    // by path; false keeps directory order
    bool sort = true;
};
using list_files_ft = std::function<std::vector<path_t> (const list_files_t&)>;

// path -> the deferred read of that path
//
using make_deferred_file_content_ft = std::function<deferred_file_content_ft (const path_t&)>;

// list_files, then a deferred read for each path
//
struct list_files_with_deferred_content_t
{
    path_t root;
    std::set<path_t> extentions;
    // by path; false keeps directory order
    bool sort = true;
    // empty: read the file
    make_deferred_file_content_ft make_deferred_content = {};
};
using list_files_with_deferred_content_ft = std::function<deferred_file_contents_with_paths_t (const list_files_with_deferred_content_t&)>;

// which side of the filesystem is handing the bytes to the filter
//
enum class content_filter_direction_et
{
    READ,
    WRITE
};

// path, bytes, and direction in. bytes out. empty: read and write keep the bytes
//
using content_filter_ft = std::function<file_content_t (
    const path_t&,
    const file_content_t&,
    content_filter_direction_et
)>;

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
    list_files_ft list_files;
    list_files_with_deferred_content_ft list_files_with_deferred_content;
    // path, bytes, and direction in. bytes out. empty: read and write keep the bytes
    content_filter_ft content_filter = {};
};

// fills list_files_with_deferred_content from list_files and read
//
void assign_list_files_with_deferred_content(filesystem_t&);

// read and write pass bytes through content_filter. an empty filter leaves them as they are
//
void apply_content_filter(filesystem_t&);

}// namespace global::filesystem::abstract
