// global
#include "global/filesystem_library/filesystem.hpp"


namespace global::filesystem::abstract
{

void assign_list_files_with_deferred_content(filesystem_t& filesystem)
{
    filesystem.list_files_with_deferred_content =
        [
            list_files = filesystem.list_files,
            read = filesystem.read
        ](const list_files_with_deferred_content_t& args) -> deferred_file_contents_with_paths_t
        {
            const auto paths = list_files({
                .root = args.root,
                .extentions = args.extentions,
                .sort = args.sort
            });

            const make_deferred_file_content_ft read_content =
                [read](const path_t& path) -> deferred_file_content_ft
                {
                    return [read, path]
                    {
                        return read(path);
                    };
                };
            const auto& make_deferred_content = args.make_deferred_content
                ? args.make_deferred_content
                : read_content;

            deferred_file_contents_with_paths_t files;
            for (const auto& path : paths)
            {
                files.push_back({
                    .deferred_content = make_deferred_content(path),
                    .path = path
                });
            }
            return files;
        };
}

void apply_content_filter(filesystem_t& filesystem)
{
    const auto filter = filesystem.content_filter;
    if (!filter)
    {
        return;
    }

    const auto try_read = filesystem.try_read;
    const auto read = filesystem.read;
    const auto write = filesystem.write;

    filesystem.try_read = [filter, try_read](const path_t& path) -> file_content_ot
    {
        auto content = try_read(path);
        if (content)
        {
            *content = filter(path, *content, content_filter_direction_et::READ);
        }
        return content;
    };
    filesystem.read = [filter, read](const path_t& path)
    {
        return filter(path, read(path), content_filter_direction_et::READ);
    };
    filesystem.write = [filter, write](const path_t& path, const file_content_t& content)
    {
        write(path, filter(path, content, content_filter_direction_et::WRITE));
    };
}

}// namespace global::filesystem::abstract

namespace global::filesystem
{

std::ostream& operator<<(std::ostream& stream, const directory_compare_result_t& result)
{
    if (!result.diff.empty())
    {
        return stream << result.diff;
    }
    return stream << (result.same ? "same" : "different");
}

}// namespace global::filesystem
