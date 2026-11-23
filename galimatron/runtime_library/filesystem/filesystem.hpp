#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"

// virtual_filesystem
#include "virtual_filesystem/filesystem.hpp"

// global
#include "global/filesystem_library/filesystem.hpp"


namespace galimatron::executable::filesystem
{

namespace vf = virtual_filesystem;

// throws from this namespace use exception::domain::runtime_library
// and exception::subdomain::runtime_library::filesystem
//

// virtual_filesystem with the Galimatron path rules on read, write, remove, and create.
// logging and Boost.LEAF sit in front.
// an empty content filter leaves the bytes as they are
//
vf::filesystem_t make_filesystem(const vf::content_filter_ft& content_filter = {});

// true when path is a file or directory under path_root, not path_root itself
//
struct has_same_root_subpath_t
{
    path_t path;
    path_t path_root;
};
bool has_same_root_subpath(const vf::filesystem_t&, const has_same_root_subpath_t&);

// path relative to path_root, split into the directory and the file name
//
struct split_path_by_parts_t
{
    path_t path_root;
    path_t path;
};
gf::path_by_parts_t split_path_by_parts(const vf::filesystem_t&, const split_path_by_parts_t&);

// one path from the old parts overlaid by the new parts.
// throws: neither side has a root, or neither side has a file name
//
path_t merge_paths(const gf::path_by_parts_t& old_parts, const gf::path_by_parts_t& new_parts);

// git diff --no-index of two directories. the result carries the unified diff
//
struct compare_directories_t
{
    path_t path_1;
    path_t path_2;
};
gf::directory_compare_result_t compare_directories(const compare_directories_t&);

// regular files under root whose extension is listed.
// a missing root is an empty result. sort is by path
//
struct list_files_t
{
    path_t root;
    std::set<path_t> extentions;
    // by path; false keeps directory order
    bool sort = true;
};
std::vector<path_t> list_files(const vf::filesystem_t&, const list_files_t&);

// path -> the deferred read of that path
//
using make_deferred_file_content_ft = std::function<deferred_file_content_ft (const path_t&)>;

// list_files, then a deferred read for each path.
// an empty make_deferred_content reads the file
//
struct list_files_with_deferred_content_t
{
    path_t root;
    std::set<path_t> extentions;
    // by path; false keeps directory order
    bool sort = true;
    make_deferred_file_content_ft make_deferred_content = {};
};
deferred_file_contents_with_paths_t list_files_with_deferred_content(
    const vf::filesystem_t&,
    const list_files_with_deferred_content_t&
);

// the process filesystem, with logging and Boost.LEAF and no content filter
//
vf::filesystem_t make_runtime_filesystem();

}// namespace galimatron::executable::filesystem
