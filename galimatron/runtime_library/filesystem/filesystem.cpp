// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// virtual_filesystem
#include "virtual_filesystem/filesystem.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::executable::filesystem
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_fs);
}

namespace
{

namespace vf = virtual_filesystem;

thread_local vf::operation_et current_operation = vf::operation_et::STATUS;
thread_local path_t current_path;

// the operations that used to log. status and list stay quiet
//
void log_call(const vf::log_t& log)
{
    current_operation = log.operation;
    current_path = log.path;
    switch (log.operation)
    {
    case vf::operation_et::TRY_READ:
        fLogmeD("try read file '{}'", log.path.string());
        break;
    case vf::operation_et::READ:
        fLogmeD("read file '{}'", log.path.string());
        break;
    case vf::operation_et::WRITE:
        fLogmeD("write file '{}'", log.path.string());
        break;
    case vf::operation_et::REMOVE:
        fLogmeD("remove '{}'", log.path.string());
        break;
    case vf::operation_et::CREATE_DIRECTORY:
    case vf::operation_et::CREATE_DIRECTORIES:
        fLogmeD("create directory '{}'", log.path.string());
        break;
    default:
        break;
    }
}

// filesystem_error becomes a Boost.LEAF exception.
// read and write keep io_operation_et. every other call carries the message
//
void fail(const vf::fs::filesystem_error& error)
{
    const auto path = error.path1().empty() ? current_path : error.path1();
    switch (current_operation)
    {
    case vf::operation_et::READ:
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem{},
            exception::details::io_operation_et::READ,
            exception::e_path_info_for(path)
        );
    case vf::operation_et::WRITE:
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem{},
            exception::details::io_operation_et::WRITE,
            exception::e_path_info_for(path)
        );
    default:
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem{},
            exception::e_what_info{ std::string{ error.what() } },
            exception::e_path_info_for(path)
        );
    }
}

}// namespace

// true when path is a file or directory under path_root, not path_root itself
//
bool has_same_root_subpath(
    const vf::filesystem_t& files,
    const has_same_root_subpath_t& args
)
{
    const auto relative = vf::relative(files, args.path, args.path_root);
    return !relative.empty() && relative.native()[0] != '.';
}

// path relative to path_root, split into the directory and the file name
//
gf::path_by_parts_t split_path_by_parts(
    const vf::filesystem_t& files,
    const split_path_by_parts_t& args
)
{
    const auto& path_root = args.path_root;
    const auto& path = args.path;
    const auto without_filename = path_t{ path }.remove_filename();
    const auto relative = vf::relative(files, without_filename, path_root);
    const auto filename = path.filename();
    const auto name = filename.stem().empty()
        ? path_ot{}
        : path_ot{ filename.stem() };
    const auto extension = filename.extension().empty()
        ? path_ot{}
        : path_ot{ filename.extension() };
    return {
        .path = path,
        .root = path_root,
        .relative_and_filename = relative / filename,
        .relative = relative,
        .filename = {
            .prefix = {},
            .name = name,
            .suffix = {},
            .extension = extension,
            .delim = {}
        }
    };
}

// one path from the old parts overlaid by the new parts.
// throws: neither side has a root, or neither side has a file name
//
path_t merge_paths(const gf::path_by_parts_t& old_parts, const gf::path_by_parts_t& new_parts)
{
    const auto missing_part = [](const std::string& what)
    {
        leaf::throw_exception(
            exception::domain::runtime_library{},
            exception::subdomain::runtime_library::filesystem{},
            exception::e_what_info{ what }
        );
    };
    return [&] -> path_t
    {
        const auto& [
            _,
            old_root,
            old_relative_and_filename,
            old_relative,
            old_filename
        ] = old_parts;
        const auto& [
            __,
            new_root,
            new_relative_and_filename,
            new_relative,
            new_filename
        ] = new_parts;

        const auto finally_root = [&] -> path_t
        {
            if (new_root)
            {
                return *new_root;
            }
            else if (old_root)
            {
                return *old_root;
            }
            missing_part("root must be present in old or new path");
            return {};
        }();

        if (new_relative_and_filename)
        {
            return finally_root / *new_relative_and_filename;
        }

        const auto finally_relative = [&] -> path_t
        {
            if (new_relative)
            {
                return *new_relative;
            }
            else if (old_relative)
            {
                return *old_relative;
            }
            return {};
        }();

        const auto& [old_prefix, old_name, old_suffix, old_extension, old_delim] = old_filename;
        const auto& [new_prefix, new_name, new_suffix, new_extension, new_delim] = new_filename;

        const auto finally_stem = [&] -> std::string
        {
            std::list<std::string> parts;

            if (new_prefix)
            {
                parts.push_back(new_prefix->string());
            }
            else if (old_prefix)
            {
                parts.push_back(old_prefix->string());
            }

            if (new_name)
            {
                parts.push_back(new_name->string());
            }
            else if (old_name)
            {
                parts.push_back(old_name->string());
            }
            else
            {
                missing_part("name must be present in old or new path");
            }

            if (new_suffix)
            {
                parts.push_back(new_suffix->string());
            }
            else if (old_suffix)
            {
                parts.push_back(old_suffix->string());
            }

            const auto finally_delim = [&] -> std::string
            {
                if (new_delim)
                {
                    return new_delim->string();
                }
                else if (old_delim)
                {
                    return old_delim->string();
                }
                return {};
            }();

            return boost::algorithm::join(parts, finally_delim);
        }();

        const auto finally_extention = [&] -> std::string
        {
            if (new_extension)
            {
                return new_extension->string();
            }
            else if (old_extension)
            {
                return old_extension->string();
            }
            return {};
        }();

        return finally_root
            / finally_relative
            / (finally_stem + finally_extention);
    }().lexically_normal();
}

std::atomic<std::uint64_t> compare_sequence = 0;

// git diff --no-index. the text is kept and the console stays quiet
//
gf::directory_compare_result_t compare_directories(const compare_directories_t& args)
{
    const auto output = fs::temp_directory_path() / std::format(
        "galimatron-compare-{}-{}.txt",
        std::chrono::steady_clock::now().time_since_epoch().count(),
        compare_sequence.fetch_add(1)
    );
    const auto command = std::format(
        "git diff --no-index \"{}\" \"{}\" > \"{}\" 2>&1",
        args.path_1.string(),
        args.path_2.string(),
        output.string()
    );
    const auto status = std::system(command.c_str());

    std::string text;
    {
        std::ifstream input(output, std::ios::in | std::ios::binary);
        if (input)
        {
            input.seekg(0, std::ios::end);
            const auto size = input.tellg();
            if (size > 0)
            {
                text.resize(static_cast<std::size_t>(size));
                input.seekg(0, std::ios::beg);
                input.read(text.data(), size);
            }
        }
    }
    fs::remove(output);

    gf::directory_compare_result_t result;
    result.same = status == 0;
    if (!result.same)
    {
        result.diff = text.empty()
            ? std::format("git diff --no-index exited {}", status)
            : std::move(text);
    }
    return result;
}

// regular files under root whose extension is listed.
// a missing root is an empty result. sort is by path
//
std::vector<path_t> list_files(
    const vf::filesystem_t& files,
    const list_files_t& args
)
{
    if (!vf::exists(files, args.root))
    {
        return {};
    }

    const auto& extentions = args.extentions;
    auto entries = files.list({
        .root = args.root,
        .options = {
            .recursive = true
        }
    });
    auto paths =
        entries
        | ranges::views::filter([&extentions](const vf::entry_t& entry)
        {
            return entry.status.type() == fs::file_type::regular
                && extentions.contains(entry.path.extension());
        })
        | ranges::views::transform([](const vf::entry_t& entry)
        {
            return entry.path;
        })
        | ranges::to_vector;
    if (args.sort)
    {
        paths |= ranges::actions::sort;
    }
    return paths;
}

// list_files, then a deferred read for each path.
// an empty make_deferred_content reads the file
//
deferred_file_contents_with_paths_t list_files_with_deferred_content(
    const vf::filesystem_t& files,
    const list_files_with_deferred_content_t& args
)
{
    const auto paths = list_files(files, {
        .root = args.root,
        .extentions = args.extentions,
        .sort = args.sort
    });

    const make_deferred_file_content_ft read_content =
        [&files](const path_t& path) -> deferred_file_content_ft
        {
            return [&files, path]
            {
                return make_file_content(files.read(path));
            };
        };
    const auto& make_deferred_content = args.make_deferred_content
        ? args.make_deferred_content
        : read_content;

    return
        paths
        | ranges::views::transform([&](const path_t& path)
        {
            return deferred_file_content_with_path_t{
                .deferred_content = make_deferred_content(path),
                .path = path
            };
        })
        | ranges::to_vector
        ;
}

namespace
{
// absolute, then lexically normal
//
path_t normalized(const vf::absolute_ft& absolute, const path_t& path)
{
    return absolute(path).lexically_normal();
}
}// namespace

vf::filesystem_t make_filesystem(const vf::content_filter_ft& content_filter)
{
    auto filesystem = vf::make_filesystem({
        .logger = [](const vf::log_t& log)
        {
            log_call(log);
        },
        .on_error = [](const vf::fs::filesystem_error& error)
        {
            fail(error);
        },
        .content_filter = content_filter
    });

    const auto absolute = filesystem.absolute;
    const auto try_read = filesystem.try_read;
    const auto read = filesystem.read;
    const auto write = filesystem.write;
    const auto remove = filesystem.remove;
    const auto create_directories = filesystem.create_directories;

    filesystem.absolute = [absolute](const path_t& path)
    {
        return normalized(absolute, path);
    };
    filesystem.try_read = [try_read, absolute](const path_t& path)
    {
        return try_read(normalized(absolute, path));
    };
    filesystem.read = [read, absolute](const path_t& path)
    {
        return read(normalized(absolute, path));
    };
    filesystem.write = [write, create_directories, absolute](const vf::write_t& args)
    {
        const auto path = normalized(absolute, args.path);
        create_directories(path.parent_path());
        write({
            .path = path,
            .bytes = args.bytes
        });
    };
    filesystem.remove = [remove, absolute](const path_t& path)
    {
        return remove(normalized(absolute, path));
    };
    filesystem.create_directory = [create_directories, absolute](const path_t& path)
    {
        return create_directories(normalized(absolute, path));
    };
    filesystem.create_directories = [create_directories, absolute](const path_t& path)
    {
        return create_directories(normalized(absolute, path));
    };

    return filesystem;
}

vf::filesystem_t make_runtime_filesystem()
{
    return make_filesystem();
}

}// namespace galimatron::executable::filesystem
