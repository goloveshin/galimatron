#pragma once

// global
#include "global/common_library/type_declaration.hpp"


namespace global::filesystem
{

// A path represented as parts.
//
// /tmp/user/root_of_group/subpath_1/subpath_2/name_of_my_xml.xml
// ----- root -------------
//                         ---- relative ------
//                                             ---- filename ----
//                         ---- relative and filename -----------
struct filename_by_parts_t
{
    path_ot prefix;
    path_ot name;
    path_ot suffix;
    path_ot extension;
    path_ot delim;
};

struct path_by_parts_t
{
    path_ot path;
    path_ot root;

    // relative + filename
    path_ot relative_and_filename;

    path_ot relative;
    filename_by_parts_t filename;
};
using path_by_parts_ot = std::optional<path_by_parts_t>;
using split_path_by_parts_ft = std::function<path_by_parts_ot (const path_t&)>;

// git diff --no-index of two directories.
// same is true when the trees match. diff is the unified diff when they do not
//
struct directory_compare_result_t
{
    bool same = false;
    std::string diff;

    explicit operator bool() const
    {
        return same;
    }
};

// the unified diff, so a failed CHECK prints it
//
std::ostream& operator<<(std::ostream& stream, const directory_compare_result_t& result);

}// namespace global::filesystem

// include only from global/filesystem_library/filesystem.hpp
#include "global/filesystem_library/detail/filesystem_abstract.hpp"
#include "global/filesystem_library/detail/filesystem_physically.hpp"
