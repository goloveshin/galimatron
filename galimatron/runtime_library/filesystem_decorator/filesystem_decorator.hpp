#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"
#include "galimatron/runtime_library/filesystem_decorator/detail/place_container.hpp"


namespace galimatron::executable::filesystem_decorator
{

// places on disk: reads, writes, and removes files by the place rules
//
class filesystem_decorator_t
{
public:
    // testing: yes (unit)
    //
    struct config_t
    {
        gf::abstract::filesystem_t filesystem;
    };
    filesystem_decorator_t(const config_t& config);

    filesystem_decorator_t(const filesystem_decorator_t&) = delete;
    filesystem_decorator_t& operator=(const filesystem_decorator_t&) = delete;
    filesystem_decorator_t(filesystem_decorator_t&&) = delete;
    filesystem_decorator_t& operator=(filesystem_decorator_t&&) = delete;

public:
    // testing: yes (unit)
    //
    place_id_t add_place(const place_t&);

    // testing: yes (unit)
    //
    const place_t& get_place(const place_id_t&);

    // testing: yes (unit)
    //
    path_t make_path(const place_id_t&, const path_t& suffix);

    // 1. creates the directory of each place
    // 2. with remove.at_begin removes files of the place extensions
    // 3. with not_rewrited_at_end remembers files that will not be rewritten
    //
    // testing: yes (unit)
    //
    void do_jobs_at_begin();

    // 1. flushes the deferred write to disk
    // 2. removes files that were never rewritten
    //
    // testing: yes (unit)
    //
    void do_jobs_at_end();

public:
    // testing: yes (unit)
    //
    gf::path_by_parts_t split_path_by_parts(const path_t&);

    // testing: yes (unit)
    //
    deferred_file_contents_with_paths_t get_place_content(const place_id_t);

    struct printout_result_t
    {
        path_t finally_path;
    };

    // testing: yes (unit)
    //
    struct printout_by_place_t
    {
        place_id_t place;
        gf::path_by_parts_t path;
        gf::path_by_parts_t replace_by;
        file_content_t content;
    };
    printout_result_t printout_to_place(const printout_by_place_t&);

    // writes a file by the place-relative path
    //
    // testing: yes (unit)
    //
    printout_result_t printout_to_place(
        const place_id_t& place,
        const path_t& relative_path,
        const file_content_t& content
    );

    // testing: yes (unit)
    //
    struct printout_by_alias_t
    {
        place_t::string_alias_t string_alias;
        gf::path_by_parts_t path;
        gf::path_by_parts_t replace_by;
        file_content_t content;
    };
    printout_result_t printout_by_alias(const printout_by_alias_t&);

    // writes a file by the place alias and the relative path.
    // the alias must name exactly one place, otherwise exception::domain::runtime_library, exception::subdomain::runtime_library::filesystem_decorator
    //
    // testing: yes (unit)
    //
    printout_result_t printout_by_alias(
        const place_t::string_alias_t& string_alias,
        const path_t& relative_path,
        const file_content_t& content
    );

    struct unmangle_saxon_uri_t
    {
        place_t::string_alias_t string_alias;
        path_t relative_path;
    };
    // a parsed @@alias@@path@@, or the path after a file URI prefix is removed
    //
    using unmangled_saxon_uri_t = std::variant<unmangle_saxon_uri_t, path_t>;
    // parses @@alias@@relative_path@@
    // a file URI is returned as the path without the file: prefix
    // any other string is returned as a path unchanged
    //
    // testing: yes (unit)
    //
    unmangled_saxon_uri_t unmangled_saxon_uri(const std::string&);

private:
    struct printout_t
    {
        place_container::container_t::record_t& place;
        gf::path_by_parts_t path;
        gf::path_by_parts_t replace_by;
        file_content_t content;
    };
    printout_result_t printout(const printout_t&);

private:
    const config_t config_;
    const gf::abstract::filesystem_t& filesystem_;
    place_container::container_t places_;
};

}// namespace galimatron::executable::filesystem_decorator
