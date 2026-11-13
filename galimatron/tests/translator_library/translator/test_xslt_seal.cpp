// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_xslt_seal")
{
    // .gal and principal xml each have their own directory
    auto run = [](const path_t& root)
    {
        translate_production(root);

        const auto filesystem = executable::filesystem::physically::make_filesystem();
        CHECK(filesystem.compare_directories({
            .path_1 = root / test_folders::actual_gal,
            .path_2 = root / test_folders::expected_gal
        }));
        CHECK(filesystem.compare_directories({
            .path_1 = root / test_folders::actual_xml,
            .path_2 = root / test_folders::expected_xml
        }));
    };

    const auto root = path_to_galimatron_test_data / "translator_library/xslt_seal";

    SUBCASE("a sealed tag is reassigned in a new version")
    {
        run(root / "reassign");
    }

    SUBCASE("a change under a later hash is reassigned after that hash")
    {
        run(root / "inherit");
    }

    SUBCASE("a sealed change is written into the open version")
    {
        run(root / "open");
    }

    SUBCASE("a change in the open version stays there")
    {
        run(root / "present");
    }

    SUBCASE("an xslt that changes nothing adds no version")
    {
        run(root / "unchanged");
    }
}

}

}// namespace galimatron
