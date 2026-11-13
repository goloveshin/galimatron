// galimatron
#include "galimatron/runtime_library/filesystem_decorator/filesystem_decorator.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron::executable::filesystem_decorator
{

namespace
{

struct fake_filesystem_t
{
    std::map<path_t, file_content_t> files;
    std::set<path_t> directories;
};

struct add_place_t
{
    path_t root;
    std::set<path_t> extentions;
    place_t::name_t name = "place";
    place_t::string_alias_ot string_alias;
    place_t::read_t read = {};
    place_t::write_t write = {};
    place_t::remove_t remove = {};
};

filesystem_decorator_t make_decorator(fake_filesystem_t& fake)
{
    gf::abstract::filesystem_t filesystem
    {
        .exist = [&](const auto& path)
        {
            return fake.files.find(path) != fake.files.end()
                || fake.directories.find(path) != fake.directories.end();
        },
        .read = [&](const auto& path)
        {
            return fake.files.at(path);
        },
        .write = [&](const auto& path, const auto& content)
        {
            fake.files[path] = content;
            fake.directories.insert(path.parent_path());
        },
        .remove = [&](const auto& path)
        {
            fake.files.erase(path);
        },
        .create_directory = [&](const auto& path)
        {
            fake.directories.insert(path);
        },
        .list_files = [&](const auto& args)
        {
            std::vector<path_t> paths;
            for (const auto& [file_path, content] : fake.files)
            {
                (void)content;
                const auto relative = file_path.lexically_relative(args.root);
                if (relative.empty() || *relative.begin() == path_t(".."))
                {
                    continue;
                }
                if (!args.extentions.contains(file_path.extension()))
                {
                    continue;
                }
                paths.push_back(file_path);
            }
            if (args.sort)
            {
                paths |= ranges::actions::sort;
            }
            return paths;
        }
    };

    gf::abstract::assign_list_files_with_deferred_content(filesystem);

    return filesystem_decorator_t(
    {
        .filesystem = filesystem
    });
}

place_id_t add_place(filesystem_decorator_t& decorator, const add_place_t& args)
{
    return decorator.add_place(
    {
        .name = args.name,
        .string_alias = args.string_alias,
        .root = args.root,
        .extentions = args.extentions,
        .read = args.read,
        .write = args.write,
        .remove = args.remove
    });
}

}// namespace

TEST_SUITE("galimatron/runtime_library/filesystem_decorator" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("add_place/get_place and split_path_by_parts")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    const auto root = path_t("/fake/root");
    const auto place_id = add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt", ".xml" },
        .name = "input"
    });

    const auto& place = decorator.get_place(place_id);
    CHECK(place.root == root);
    CHECK(place.extentions == std::set<path_t>{ ".txt", ".xml" });

    const auto path = root / "sub" / "dir" / "file.txt";
    const auto actual = decorator.split_path_by_parts(path);

    CHECK(actual.root == root);
    CHECK(actual.relative == "sub/dir/");
    CHECK(actual.relative_and_filename == "sub/dir/file.txt");
    CHECK(actual.filename.name.has_value());
    CHECK(*actual.filename.name == path_t("file"));
    CHECK(actual.filename.extension.has_value());
    CHECK(*actual.filename.extension == path_t(".txt"));
}

TEST_CASE("printout_to_place writes through fake backend")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    const auto root = path_t("/fake/root");
    const auto place_id = add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" }
    });

    const auto content = make_file_content("hello decorator");
    const auto result = decorator.printout_to_place(place_id, "sub/hello.txt", content);

    CHECK(result.finally_path == root / "sub/hello.txt");
    CHECK(fake.files.find(result.finally_path) != fake.files.end());
    CHECK(*fake.files[result.finally_path] == "hello decorator");
}

TEST_CASE("printout_to_place once mode rejects duplicates")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    const auto root = path_t("/fake/root");
    const auto place_id = add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" },
        .write = { .once = true }
    });

    decorator.printout_to_place(place_id, "once.txt", make_file_content("first"));
    CHECK_THROWS(decorator.printout_to_place(place_id, "once.txt", make_file_content("second")));
}

TEST_CASE("printout_by_alias and unmangle_saxon_uri")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    const auto root = path_t("/fake/root");
    add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" },
        .string_alias = "alias"
    });

    const auto result = decorator.printout_by_alias("alias", "nested/out.txt", make_file_content("alias output"));

    CHECK(result.finally_path == root / "nested/out.txt");
    CHECK(fake.files.find(result.finally_path) != fake.files.end());
    CHECK(*fake.files[result.finally_path] == "alias output");

    const auto unmangled = decorator.unmangled_saxon_uri("@@alias@@nested/out.txt@@");
    const auto* parsed = std::get_if<filesystem_decorator_t::unmangle_saxon_uri_t>(&unmangled);
    REQUIRE(parsed);
    CHECK(parsed->string_alias == "alias");
    CHECK(parsed->relative_path == "nested/out.txt");

    const auto plain = decorator.unmangled_saxon_uri("not-a-saxon-uri");
    const auto* as_path = std::get_if<path_t>(&plain);
    REQUIRE(as_path);
    CHECK(*as_path == path_t{ "not-a-saxon-uri" });

    const auto file_uri = decorator.unmangled_saxon_uri("file:///D:/work/out.xml");
    const auto* from_file_uri = std::get_if<path_t>(&file_uri);
    REQUIRE(from_file_uri);
    CHECK(*from_file_uri == path_t{ "D:/work/out.xml" });

    const auto file_scheme = decorator.unmangled_saxon_uri("file:/D:/work/out.xml");
    const auto* from_file_scheme = std::get_if<path_t>(&file_scheme);
    REQUIRE(from_file_scheme);
    CHECK(*from_file_scheme == path_t{ "D:/work/out.xml" });

    const auto file_host = decorator.unmangled_saxon_uri("file://D:/work/out.xml");
    const auto* from_file_host = std::get_if<path_t>(&file_host);
    REQUIRE(from_file_host);
    CHECK(*from_file_host == path_t{ "/D:/work/out.xml" });
}

TEST_CASE("make_path")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    const auto root = path_t("/fake/root");
    const auto place_id = add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" }
    });

    const auto suffix = path_t("sub") / "file.txt";
    const auto expected = gf::physically::make_filesystem().to_absolute(root / suffix);
    CHECK(decorator.make_path(place_id, suffix) == expected);
}

TEST_CASE("do_jobs_at_begin removes files of the place extensions")
{
    fake_filesystem_t fake;
    const auto root = path_t("/fake/root");
    const auto gone = root / "gone.txt";
    const auto stays = root / "stay.xml";
    fake.files[gone] = make_file_content("gone");
    fake.files[stays] = make_file_content("stay");

    auto decorator = make_decorator(fake);
    add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" },
        .remove = { .at_begin = true }
    });

    decorator.do_jobs_at_begin();

    CHECK(fake.directories.find(root) != fake.directories.end());
    CHECK(fake.files.find(gone) == fake.files.end());
    CHECK(*fake.files.at(stays) == "stay");
}

TEST_CASE("do_jobs_at_end flushes a delayed write and removes a file that was not rewritten")
{
    fake_filesystem_t fake;
    const auto root = path_t("/fake/root");
    const auto kept = root / "kept.txt";
    const auto dropped = root / "dropped.txt";
    fake.files[kept] = make_file_content("old-kept");
    fake.files[dropped] = make_file_content("old-dropped");

    auto decorator = make_decorator(fake);
    const auto place_id = add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" },
        .write = { .delayed_to_demand = true },
        .remove = { .not_rewrited_at_end = true }
    });

    decorator.do_jobs_at_begin();

    const auto result = decorator.printout_to_place(place_id, "kept.txt", make_file_content("new-kept"));
    CHECK(result.finally_path == kept);
    CHECK(*fake.files.at(kept) == "old-kept");

    decorator.do_jobs_at_end();

    CHECK(*fake.files.at(kept) == "new-kept");
    CHECK(fake.files.find(dropped) == fake.files.end());
}

TEST_CASE("get_place_content")
{
    fake_filesystem_t fake;
    const auto root = path_t("/fake/root");
    const auto text = root / "a.txt";
    const auto nested = root / "nested" / "c.txt";
    const auto other = root / "skip.xml";
    const auto outside = path_t("/fake/other") / "no.txt";
    fake.files[text] = make_file_content("aaa");
    fake.files[nested] = make_file_content("ccc");
    fake.files[other] = make_file_content("xml");
    fake.files[outside] = make_file_content("no");

    auto decorator = make_decorator(fake);
    const auto place_id = add_place(decorator,
    {
        .root = root,
        .extentions = { ".txt" }
    });

    std::map<path_t, std::string> actual;
    for (const auto& item : decorator.get_place_content(place_id))
    {
        actual.emplace(item.path, *item.deferred_content());
    }

    CHECK(actual.size() == 2);
    CHECK(actual.at(text) == "aaa");
    CHECK(actual.at(nested) == "ccc");
}

TEST_CASE("a repeated place alias cannot be written by alias")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    add_place(decorator,
    {
        .root = "/fake/one",
        .extentions = { ".hpp" },
        .string_alias = "generated-dto"
    });
    add_place(decorator,
    {
        .root = "/fake/two",
        .extentions = { ".hpp" },
        .string_alias = "generated-dto"
    });

    bool caught = false;
    print_leaf_exception(
        [&]
        {
            decorator.printout_by_alias("generated-dto", "out.hpp", make_file_content("dto"));
        },
        false,
        [&](
            const exception::domain::runtime_library&,
            const exception::subdomain::runtime_library::filesystem_decorator&,
            const exception::e_what_info& info
        )
        {
            caught = true;
            CHECK(info.what == "place alias 'generated-dto' names more than one place");
        }
    );
    CHECK(caught);
}

TEST_CASE("place roots cannot be nested")
{
    fake_filesystem_t fake;
    auto decorator = make_decorator(fake);

    add_place(decorator,
    {
        .root = "/fake/generated/",
        .extentions = { ".hpp", ".cpp" },
        .string_alias = "generated"
    });
    add_place(decorator,
    {
        .root = "/fake/generated",
        .extentions = { ".txt" }
    });
    add_place(decorator,
    {
        .root = "/fake/generated_conversion/",
        .extentions = { ".hpp", ".cpp" },
        .string_alias = "generated-conversion"
    });

    bool caught = false;
    print_leaf_exception(
        [&]
        {
            add_place(decorator,
            {
                .root = "/fake/generated/conversion/",
                .extentions = { ".hpp", ".cpp" },
                .string_alias = "generated-nested"
            });
        },
        false,
        [&](
            const exception::domain::runtime_library&,
            const exception::subdomain::runtime_library::filesystem_decorator&,
            const exception::e_what_info& info
        )
        {
            caught = true;
            CHECK(info.what == "place root '/fake/generated/conversion' is nested in '/fake/generated'");
        }
    );
    CHECK(caught);

    fake_filesystem_t reverse_fake;
    auto reverse = make_decorator(reverse_fake);
    add_place(reverse,
    {
        .root = "/fake/generated/conversion",
        .extentions = { ".hpp", ".cpp" },
        .string_alias = "generated-nested"
    });

    bool reverse_caught = false;
    print_leaf_exception(
        [&]
        {
            add_place(reverse,
            {
                .root = "/fake/generated/",
                .extentions = { ".hpp", ".cpp" },
                .string_alias = "generated"
            });
        },
        false,
        [&](
            const exception::domain::runtime_library&,
            const exception::subdomain::runtime_library::filesystem_decorator&,
            const exception::e_what_info& info
        )
        {
            reverse_caught = true;
            CHECK(info.what == "place root '/fake/generated/conversion' is nested in '/fake/generated'");
        }
    );
    CHECK(reverse_caught);
}

}

}// namespace galimatron::executable::filesystem_decorator
