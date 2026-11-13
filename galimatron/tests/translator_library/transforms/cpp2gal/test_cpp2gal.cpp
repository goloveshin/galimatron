// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library/transforms/cpp2gal" * galimatron::test_axes::mark(galimatron::test_axes::translator_slice))
{

// gal2cpp then cpp2gal
// a parsed tag keeps the user's spaces, line breaks, and the comment after the semicolon
// an ordering is reprinted on one line when that line fits in 100 columns
//
TEST_CASE("test_cpp2gal")
{
    auto run = [](const path_t& root)
    {
        translate_slice(root, {
            .gal2cpp = true,
            .cpp2cpp_validate = false,
            .cpp2cpp_wizard = false,
            .cpp2xml = false,
            .xml2cpp = false,
            .cpp2gal = true,
            .cpp2post_cpp = false,
            .post_cpp2post_xml = false
        });

        const auto filesystem = executable::filesystem::physically::make_filesystem();
        CHECK(filesystem.compare_directories({
            .path_1 = root / test_folders::actual_gal,
            .path_2 = root / test_folders::expected_gal
        }));
    };

    const auto root = path_to_galimatron_test_data / "translator_library/cpp2gal";

    SUBCASE("language roundtrip keeps the written statements")
    {
        run(root / "language");
    }

    SUBCASE("three uuid layouts stay as written")
    {
        run(root / "formatting");
    }
}

// a tag or an ordering with no stored source text
// flat text stays on one line through 100 columns, counting the indent
// a nested tag value breaks, and a flat line past 100 columns breaks
//
TEST_CASE("default tag layout")
{
    const auto root = path_to_galimatron_test_data / "translator_library/cpp2gal/default_layout";

    translate_slice(root, {
        .gal2cpp = true,
        .cpp2cpp_validate = false,
        .cpp2cpp_wizard = false,
        .cpp2xml = false,
        .xml2cpp = false,
        .cpp2gal = true,
        .cpp2post_cpp = false,
        .post_cpp2post_xml = false,
        .drop_tag_user_formatting = true
    });

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_gal,
        .path_2 = root / test_folders::expected_gal
    }));
}

// gal2cpp then cpp2gal, and the same trip through xml
// a raw string is printed from its delimiter and its trimmed text
//
TEST_CASE("raw string roundtrip")
{
    const auto root = path_to_galimatron_test_data / "translator_library/cpp2gal/raw_string";

    auto run = [&](const bool through_xml)
    {
        translate_slice(root, {
            .gal2cpp = true,
            .cpp2cpp_validate = false,
            .cpp2cpp_wizard = false,
            .cpp2xml = through_xml,
            .xml2cpp = through_xml,
            .cpp2gal = true,
            .cpp2post_cpp = false,
            .post_cpp2post_xml = false
        });
    };

    SUBCASE("gal2gal")
    {
        run(false);
    }

    SUBCASE("through xml")
    {
        run(true);
    }

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_gal,
        .path_2 = root / test_folders::expected_gal
    }));
}

// a comment before an aggregate, a version, and a tag stays where it was written
//
TEST_CASE("user comments roundtrip through cpp2gal")
{
    const auto root = path_to_galimatron_test_data / "translator_library/cpp2gal/user_comments";

    translate_slice(root, {
        .gal2cpp = true,
        .cpp2cpp_validate = false,
        .cpp2cpp_wizard = false,
        .cpp2xml = false,
        .xml2cpp = false,
        .cpp2gal = true,
        .cpp2post_cpp = false,
        .post_cpp2post_xml = false
    });

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_gal,
        .path_2 = root / test_folders::expected_gal
    }));
}

}

}// namespace galimatron
