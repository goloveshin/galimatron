// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_wizard_delete_hash")
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

    const auto root = path_to_galimatron_test_data / "translator_library/wizard_delete_hash";

    SUBCASE("versions")
    {
        run(root / "versions");
    }

    SUBCASE("last version")
    {
        run(root / "last");
    }

    SUBCASE("force removes a stored hash that does not match")
    {
        run(root / "force");
    }
}

}

}// namespace galimatron
