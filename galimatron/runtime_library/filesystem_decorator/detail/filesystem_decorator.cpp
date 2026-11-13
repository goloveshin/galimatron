// galimatron
#include "galimatron/runtime_library/filesystem_decorator/filesystem_decorator.hpp"
#include "galimatron/common_library/exception_augment.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::executable::filesystem_decorator
{

using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_fsd);
}


namespace
{
// to_absolute, split_path_by_parts, and merge_paths are taken from global::filesystem::physically
// the other calls are empty and throw std::bad_function_call
//
const gf::physically::filesystem_t& path_filesystem()
{
    static const auto filesystem = []
    {
        const auto full = gf::physically::make_filesystem();
        return gf::physically::filesystem_t{
            .to_absolute = full.to_absolute,
            .split_path_by_parts = full.split_path_by_parts,
            .merge_paths = full.merge_paths
        };
    }();
    return filesystem;
}
}// namespace


filesystem_decorator_t::filesystem_decorator_t(const config_t& config)
    : config_(config)
    , filesystem_(config_.filesystem)
{
}

place_id_t filesystem_decorator_t::add_place(const place_t& args)
{
    return places_.add_place(args);
}

const place_t& filesystem_decorator_t::get_place(const place_id_t& place_id)
{
    return places_.get_place(place_id);
}

path_t filesystem_decorator_t::make_path(const place_id_t& place_id, const path_t& suffix)
{
    return path_filesystem().to_absolute(get_place(place_id).root / suffix);
}

void filesystem_decorator_t::do_jobs_at_begin()
{
    fLogmeD("filesystem_decorator: begin do jobs at begin");

    for (const auto& const_place : places_)
    {
        auto& place = places_.to_non_const(const_place);

        filesystem_.create_directory(place.root);

        auto list_files = [&]
        {
            return filesystem_.list_files({
                .root = place.root,
                .extentions = place.extentions
            });
        };

        if (place.remove.at_begin)
        {
            for (const auto& path : list_files())
            {
                fLogmeD("remove file from disk: '{}' (remove-at-begin is enabled)", path.string());
                filesystem_.remove(path);
            }
        }
        else
        {
            fLogmeD("files not removed from disk in place '{}' (remove-at-begin is disabled)", place.name);
        }

        if (place.remove.not_rewrited_at_end)
        {
            for (auto& entry : place.files)
            {
                entry.second.remove_at_end = false;
            }
            for (const auto& path : list_files())
            {
                place.files[path].remove_at_end = true;
            }
        }
    }

    fLogmeD("filesystem_decorator: end do jobs at begin");
}

void filesystem_decorator_t::do_jobs_at_end()
{
    fLogmeD("filesystem_decorator: begin do jobs at end");

    for (const auto& const_place : places_)
    {
        auto& place = places_.to_non_const(const_place);

        bool removed = false;
        for (auto it = place.files.begin(); it != place.files.end();)
        {
            const auto& path = it->first;
            auto& file = it->second;

            if (file.pending_write)
            {
                fLogmeD("write file to disk: '{}' (delayed write flush)", path.string());
                filesystem_.write(path, *file.content);
                file.pending_write = false;
            }

            if (file.remove_at_end)
            {
                fLogmeD("remove file from disk: '{}' (file was not rewritten)", path.string());
                filesystem_.remove(path);
                removed = true;
                it = place.files.erase(it);
                continue;
            }

            ++it;
        }
        if (!removed)
        {
            fLogmeD("files not removed from disk in place '{}' (all files were rewritten or tracking is disabled)", place.name);
        }
    }

    fLogmeD("filesystem_decorator: end do jobs at end");
}

gf::path_by_parts_t filesystem_decorator_t::split_path_by_parts(const path_t& path)
{
    if (const auto place_by_path = places_.try_find_by_path(path))
    {
        return path_filesystem().split_path_by_parts({
            .path_root = place_by_path->get().root,
            .path = path
        });
    }

    leaf::throw_exception(
        exception::domain::runtime_library{},
        exception::subdomain::runtime_library::filesystem_decorator{},
        exception::e_what_info{ std::format(
            "cannot find place for path '{}'",
            report_path(path)
        ) },
        exception::e_path_info_for(path)
    );
}

deferred_file_contents_with_paths_t filesystem_decorator_t::get_place_content(const place_id_t place_id)
{
    auto& place = places_.find_by_place(place_id);

    return filesystem_.list_files_with_deferred_content(
    {
        .root = place.root,
        .extentions = place.extentions,
        .make_deferred_content = [&place, this](const path_t& path) -> deferred_file_content_ft
        {
            return [&place, path, this]
            {
                fLogmeD("read file from disk: '{}' (deferred content requested)", path.string());
                const auto& content = filesystem_.read(path);
                if (place.read.cache)
                {
                    auto& file = place.files[path];
                    if (!file.content)
                    {
                        file.content = content;
                        fLogmeD("cache file content after disk read: '{}' (read cache is enabled)", path.string());
                    }
                }
                return content;
            };
        }
    });
}

filesystem_decorator_t::printout_result_t filesystem_decorator_t::printout_to_place(const printout_by_place_t& args)
{
    return printout({
        .place = places_.find_by_place(args.place),
        .path = args.path,
        .replace_by = args.replace_by,
        .content = args.content
    });
}

filesystem_decorator_t::printout_result_t filesystem_decorator_t::printout_to_place(
    const place_id_t& place,
    const path_t& relative_path,
    const file_content_t& content
)
{
    return printout_to_place({
        .place = place,
        .replace_by = {
            .relative_and_filename = relative_path
        },
        .content = content
    });
}

filesystem_decorator_t::printout_result_t filesystem_decorator_t::printout_by_alias(const printout_by_alias_t& args)
{
    return printout({
        .place = places_.find_by_alias(args.string_alias),
        .path = args.path,
        .replace_by = args.replace_by,
        .content = args.content
    });
}

filesystem_decorator_t::printout_result_t filesystem_decorator_t::printout_by_alias(
    const place_t::string_alias_t& string_alias,
    const path_t& relative_path,
    const file_content_t& content
)
{
    return printout_by_alias({
        .string_alias = string_alias,
        .replace_by = {
            .relative_and_filename = relative_path
        },
        .content = content
    });
}

filesystem_decorator_t::printout_result_t filesystem_decorator_t::printout(const printout_t& args)
{
    auto& place = args.place;
    const auto& old_part_tmp = args.path;
    const auto& new_parts = args.replace_by;
    const auto& new_content = args.content;

    // the old path, if given, is split into parts of the place it belongs to.
    // a merge failure has no path yet
    gf::path_by_parts_t old_parts;
    if (old_part_tmp.path)
    {
        old_parts = split_path_by_parts(*old_part_tmp.path);
    }
    // the result root is always the destination place root, even if the old path was in another place
    old_parts.root = place.root;

    // the given replace_by parts replace the same-named parts of the old path.
    // no old path: the file is assembled only from replace_by and the place root
    //
    // a missing root or file name. the path is not assembled yet
    //
    const auto throw_merge = [](const std::string& what)
    {
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem_decorator{},
            exception::e_what_info{ what }
        );
    };
    const auto finally_path = path_filesystem().merge_paths({
        .old_parts = old_parts,
        .new_parts = new_parts,
        .point_1 = throw_merge,
        .point_2 = throw_merge
    });
    fLogmeD("incoming path for printout: '{}'", finally_path.string());

    auto& file = place.files[finally_path];

    // write.once: mark the path as written; the same path cannot be written a second time
    if (place.write.once)
    {
        if (file.saved)
        {
            fLogmeD("file already was saved, but 'once' mode is on");

            leaf::throw_exception(
                exception::domain::runtime_library{},
                exception::subdomain::runtime_library::filesystem_decorator{},
                exception::e_what_info{ std::format(
                    "path '{}' cannot be saved twice",
                    report_path(finally_path)
                ) },
                exception::e_path_info_for(finally_path)
            );
        }
        file.saved = true;
        fLogmeD("mark 'saved' for 'once' mode");
    }

    // the path was rewritten; do not remove it at the end of do_jobs_at_end
    file.remove_at_end = false;

    // without comparison we always write, otherwise only when the new content differs from the current one
    const bool is_need_to_do_write = [&]
    {
        if (place.write.if_content_differ)
        {
            // current content: already known, otherwise the disk, otherwise empty — the file does not exist, we write
            const auto current_content = [&] -> file_content_ot
            {
                if (file.content)
                {
                    fLogmeD("read file content from cache: '{}' (avoid disk read)", finally_path.string());
                    return file.content;
                }
                if (filesystem_.exist(finally_path))
                {
                    fLogmeD("read file from disk: '{}' (content comparison requires current content)", finally_path.string());
                    return filesystem_.read(finally_path);
                }
                fLogmeD("file not read from disk: '{}' (file does not exist)", finally_path.string());
                return {};
            }();
            const bool is_differ = current_content != new_content;
            fLogmeD("content changed: {}", is_differ);
            return is_differ;
        }
        return true;
    }();

    if (is_need_to_do_write)
    {
        file.content = new_content;
        // it will go to disk in do_jobs_at_end
        if (place.write.delayed_to_demand)
        {
            fLogmeD("file not written to disk: '{}' (write is delayed)", finally_path.string());
            file.pending_write = true;
        }
        else
        {
            fLogmeD("write file to disk: '{}' (content differs or comparison is disabled)", finally_path.string());
            filesystem_.write(finally_path, new_content);
            file.pending_write = false;
        }
    }
    else
    {
        fLogmeD("file not written to disk: '{}' (content is unchanged)", finally_path.string());
    }

    return { .finally_path = finally_path };
}

filesystem_decorator_t::unmangled_saxon_uri_t
filesystem_decorator_t::unmangled_saxon_uri(const std::string& mangled_uri)
{
    static std::regex re("@@(.+?)@@(.+?)@@");

    if (std::smatch m; std::regex_search(mangled_uri, m, re))
    {
        return unmangle_saxon_uri_t{
            .string_alias = std::string(m[1]),
            .relative_path = std::string(m[2])
        };
    }

    // file:/// and file:/ are a result-document href; the caller wants the path
    // file:// keeps the leading slash: the optional pair is taken only when a third slash follows
    static std::regex file_uri("^file:(/{2})?/");
    return path_t{ std::regex_replace(mangled_uri, file_uri, "") };
}

}// namespace galimatron::executable::filesystem_decorator
