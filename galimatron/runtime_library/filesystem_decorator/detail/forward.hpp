#pragma once

// galimatron
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/forward.hpp"


namespace galimatron::executable::filesystem_decorator
{

struct place_t
{
    using name_t = std::string;
    using string_alias_t = std::string;
    using string_alias_ot = std::optional<string_alias_t>;

    // name for the log
    name_t name;
    // place alias, for use in xslt during generation. several places may share one alias
    string_alias_ot string_alias;

    // place root
    path_t root;
    // file extensions at the place
    std::set<path_t> extentions;

    struct read_t
    {
        // cache the content if it was read once
        bool cache = true;
    };
    read_t read;

    struct write_t
    {
        // defer the write until demanded
        bool delayed_to_demand = false;

        // write according to a difference of contents
        // (current from the cache if present, otherwise it is read)
        bool if_content_differ = false;

        // writing is possible only once
        bool once = false;
    };
    write_t write;

    struct remove_t
    {
        // remove files at the start of the work
        bool at_begin = false;

        // removal of files that were not rewritten at the end of the work
        bool not_rewrited_at_end = false;
    };
    remove_t remove;
};

using place_id_t = std::size_t;

}// namespace galimatron::executable::filesystem_decorator
