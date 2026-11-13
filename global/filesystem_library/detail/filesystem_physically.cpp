// global
#include "global/filesystem_library/filesystem.hpp"


namespace global::filesystem::physically
{

namespace
{

std::atomic<std::uint64_t> compare_sequence = 0;

struct filesystem_impl_t
{
    static path_t get_relative_path_internal(const path_t& path_root, const path_t& path_without_name)
    {
        return fs::relative(path_without_name, path_root);
    }

    static bool exist(const path_t& path)
    {
        return fs::exists(path);
    }

    static bool is_regular_file(const path_t& path)
    {
        return fs::is_regular_file(path);
    }

    static ranges::any_view<fs::directory_entry> get_recursive_directory_range(const path_t& path)
    {
        fs::recursive_directory_iterator begin(path), end;
        return ranges::subrange(begin, end);
    }

    static file_content_ot try_read(const try_read_t& args)
    {
        const auto path = to_absolute(args.path);

        args.logger(path);

        std::ifstream f(path, std::ios::in | std::ios::binary);

        if (f.is_open())
        {
            const auto sz = fs::file_size(path);
            auto content = make_file_content(sz, '\0');
            f.read(content->data(), sz);
            return content;
        }
        return {};
    }

    static file_content_t read(const read_t& args)
    {
        return
            try_read({
                .path = args.path,
                .logger = args.logger
            })
            .or_else([&]{
                args.thrower(to_absolute(args.path));
                return file_content_ot{};
            })
            .value();
    }

    static void write(const write_t& args)
    {
        // a composed path may contain . or ..
        const auto path = to_absolute(args.path.lexically_normal());
        args.logger(path);
        fs::create_directories(path.parent_path());
        std::ofstream(path, std::ios::out | std::ios::binary) << *args.content;
    }

    static void remove(const remove_t& args)
    {
        const auto path = to_absolute(args.path);
        args.logger(path);
        fs::remove(path);
    }

    static void create_directory(const create_directory_t& args)
    {
        const auto path = to_absolute(args.path);
        args.logger(path);
        fs::create_directories(path);// creates everything in args.path that does not exist yet
    }

    static path_t to_absolute(const path_t& path)
    {
        return fs::absolute(path).lexically_normal();
    }

    static bool has_same_root_subpath(const has_same_root_subpath_t& args)
    {
        const auto rel = fs::relative(args.path, args.path_root);
        return !rel.empty() && rel.native()[0] != '.';
    }

    static path_t get_relative_path_without_filename(const get_relative_path_without_filename_t& args)
    {
        return get_relative_path_internal(args.path_root, path_t{args.path}.remove_filename());
    }

    static path_t get_relative_path_with_filename(const get_relative_path_with_filename_t& args)
    {
        return
            get_relative_path_without_filename({
                .path = args.path,
                .path_root = args.path_root
            })
            / args.path.filename();
    }

    static path_by_parts_t split_path_by_parts(const split_path_by_parts_t& args)
    {
        const auto& path_root = args.path_root;
        const auto& path = args.path;

        const auto relative_and_filename = get_relative_path_with_filename(
        {
            .path = path,
            .path_root = path_root
        });
        const auto relative = get_relative_path_without_filename(
        {
            .path = path,
            .path_root = path_root
        });
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
            .relative_and_filename = relative_and_filename,
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

    static path_t merge_paths(const merge_paths_t& args)
    {
        const auto& old_parts = args.old_parts;
        const auto& new_parts = args.new_parts;

        return [&] -> path_t
        {
            const auto& [_,  old_root, old_relative_and_filename, old_relative, old_filename] = old_parts;
            const auto& [__, new_root, new_relative_and_filename, new_relative, new_filename] = new_parts;

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
                args.point_1("root must be present in old or new path");
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
                    args.point_2("name must be present in old or new path");
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

    // runs git diff --no-index and keeps the text. the console stays quiet
    //
    static directory_compare_result_t compare_directories(const compare_directories_t& args)
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

        directory_compare_result_t result;
        result.same = status == 0;
        if (!result.same)
        {
            result.diff = text.empty()
                ? std::format("git diff --no-index exited {}", status)
                : std::move(text);
        }
        return result;
    }
};

}// namespace

filesystem_t make_filesystem()
{
    return {
        .exist = &filesystem_impl_t::exist,
        .is_regular_file = &filesystem_impl_t::is_regular_file,
        .get_recursive_directory_range = &filesystem_impl_t::get_recursive_directory_range,
        .try_read = &filesystem_impl_t::try_read,
        .read = &filesystem_impl_t::read,
        .write = &filesystem_impl_t::write,
        .remove = &filesystem_impl_t::remove,
        .create_directory = &filesystem_impl_t::create_directory,
        .to_absolute = &filesystem_impl_t::to_absolute,
        .has_same_root_subpath = &filesystem_impl_t::has_same_root_subpath,
        .get_relative_path_without_filename = &filesystem_impl_t::get_relative_path_without_filename,
        .get_relative_path_with_filename = &filesystem_impl_t::get_relative_path_with_filename,
        .split_path_by_parts = &filesystem_impl_t::split_path_by_parts,
        .merge_paths = &filesystem_impl_t::merge_paths,
        .compare_directories = &filesystem_impl_t::compare_directories
    };
}

}// namespace global::filesystem::physically
