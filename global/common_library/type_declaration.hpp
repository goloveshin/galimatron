#pragma once

// global
#include "global/common_library/naming_declaration.hpp"


namespace global
{

namespace fs = std::filesystem;
namespace leaf = boost::leaf;


using index_t = std::size_t;
using index_ot = std::optional<index_t>;
using indices_t = std::vector<index_t>;
using indices_ot = std::optional<indices_t>;

using bool_ot = std::optional<bool>;

using name_t = std::string;
using name_ot = std::optional<name_t>;

using string_ot = std::optional<std::string>;
using strings_t = std::deque<std::string>;

// any 'small' content
using content_t = std::string;
using content_ot = std::optional<content_t>;

// file content that can be large
using file_content_t = std::shared_ptr<content_t>;
using file_content_ot = std::optional<file_content_t>;

auto make_file_content(auto&& ... args)
{
    return std::make_shared<content_t>(std::forward<decltype(args)>(args)...);
}

// deferred retrieval of file content
using deferred_file_content_ft = std::function<file_content_t ()>;

inline bool operator==(const file_content_t& lhs, const file_content_t& rhs)
{
    if (!lhs && !rhs) return true;
    if (!lhs || !rhs) return false;
    return *lhs == *rhs;
}

using path_t = fs::path;
using path_ot = std::optional<path_t>;
using paths_t = std::list<path_t>;
using paths_ot = std::optional<paths_t>;

struct file_content_with_path_t
{
    file_content_t content;
    path_t path;
    bool operator==(const file_content_with_path_t&) const = default;
};
using file_content_with_path_ot = std::optional<file_content_with_path_t>;
using file_contents_with_paths_t = std::list<file_content_with_path_t>;
using file_contents_with_paths_ot = std::optional<file_contents_with_paths_t>;

struct deferred_file_content_with_path_t
{
    deferred_file_content_ft deferred_content;
    path_t path;
};
using deferred_file_content_with_path_ot = std::optional<deferred_file_content_with_path_t>;
using deferred_file_contents_with_paths_t = std::vector<deferred_file_content_with_path_t>;

// the file a chunk was read from
struct location_t
{
    path_ot path;
};
using location_ot = std::optional<location_t>;

}//namespace global
