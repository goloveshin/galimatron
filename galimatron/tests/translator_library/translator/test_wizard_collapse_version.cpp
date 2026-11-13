// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_wizard_collapse_version")
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

    const auto root = path_to_galimatron_test_data / "translator_library/wizard_collapse_version";

    SUBCASE("versions 1 and 2 collapse into the anonymous body owned by version 3")
    {
        run(root / "range");
    }

    SUBCASE("an anonymous body owned by a collapsed version moves with that version")
    {
        run(root / "with_anonymous");
    }

    SUBCASE("closed versions 1 and 2 collapse into the anonymous body owned by version 3")
    {
        run(root / "closed");
    }

    SUBCASE("the last assignment of a tag stays in the collapsed body")
    {
        run(root / "reassign");
    }

    SUBCASE("an aggregate whose owner is already deleted stays deleted")
    {
        run(root / "stay_deleted");
    }
}

}

}// namespace galimatron
