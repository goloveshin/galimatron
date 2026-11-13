// galimatron
#include "galimatron/translator_library/parser/gal_version.hpp"
#include "galimatron/translator_library/parser/parser.hpp"
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library/parser/gal_version" * galimatron::test_axes::mark(galimatron::test_axes::parser))
{

TEST_CASE("file name and current version")
{
    CHECK(std::string{ translator::parser::gal_version::file_name } == "gal.version");
    CHECK(translator::parser::gal_version::current == 1);
}

TEST_CASE("from_content")
{
    namespace gal_version = translator::parser::gal_version;

    SUBCASE("plain number")
    {
        CHECK(gal_version::from_content("1") == 1);
        CHECK(gal_version::from_content("2") == 2);
    }

    SUBCASE("whitespace around the number")
    {
        CHECK(gal_version::from_content("1\n") == 1);
        CHECK(gal_version::from_content(" \t1\r\n") == 1);
    }

    auto expect_bad = [](const content_t& text)
    {
        bool caught = false;

        print_leaf_exception(
            [&]
            {
                gal_version::from_content(text);
            },
            false,
            [&](
                const exception::domain::translator_library&,
                const exception::subdomain::translator_library::parser&,
                const exception::e_what_info& info
            )
            {
                caught = true;
                CHECK(info.what == "gal.version must contain a version number");
            }
        );

        CHECK(caught);
    };

    SUBCASE("empty")
    {
        expect_bad("");
        expect_bad(" \n");
    }

    SUBCASE("zero")
    {
        expect_bad("0");
    }

    SUBCASE("not a number")
    {
        expect_bad("abc");
        expect_bad("1 2");
        expect_bad("-1");
    }
}

TEST_CASE("from_input_dir")
{
    namespace gal_version = translator::parser::gal_version;

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    const auto dir = std::filesystem::temp_directory_path() / "galimatron_gal_version_unit";
    filesystem.create_directory(dir);

    const auto path = dir / std::string{ gal_version::file_name };
    if (filesystem.exist(path))
    {
        filesystem.remove(path);
    }

    SUBCASE("missing file is the current version")
    {
        CHECK(gal_version::from_input_dir(filesystem, dir) == gal_version::current);
    }

    SUBCASE("file content is the version")
    {
        filesystem.write(path, make_file_content("2\n"));
        CHECK(gal_version::from_input_dir(filesystem, dir) == 2);
        filesystem.remove(path);
    }
}

TEST_CASE("require_supported")
{
    namespace gal_version = translator::parser::gal_version;

    SUBCASE("current version")
    {
        CHECK_NOTHROW(gal_version::require_supported(gal_version::current));
    }

    SUBCASE("unknown version")
    {
        bool caught = false;

        print_leaf_exception(
            [&]
            {
                gal_version::require_supported(2);
            },
            false,
            [&](
                const exception::domain::translator_library&,
                const exception::subdomain::translator_library::parser&,
                const exception::e_what_info& info
            )
            {
                caught = true;
                CHECK(info.what == "unknown GaL version: 2");
            }
        );

        CHECK(caught);
    }
}

TEST_CASE("parse selects the current parser")
{
    namespace gal_version = translator::parser::gal_version;

    const auto content = make_file_content("wizard create version;");

    SUBCASE("version 1")
    {
        CHECK_NOTHROW(translator::parser::parse(gal_version::current, content));
    }

    SUBCASE("unknown version")
    {
        bool caught = false;

        print_leaf_exception(
            [&]
            {
                translator::parser::parse(2, content);
            },
            false,
            [&](
                const exception::domain::translator_library&,
                const exception::subdomain::translator_library::parser&,
                const exception::e_what_info& info
            )
            {
                caught = true;
                CHECK(info.what == "unknown GaL version: 2");
            }
        );

        CHECK(caught);
    }
}

}

}// namespace galimatron
