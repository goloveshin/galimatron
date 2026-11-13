// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_wizard_delete_aggregate")
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

    const auto root = path_to_galimatron_test_data / "translator_library/wizard_delete_aggregate";

    SUBCASE("a list writes delete; into each last version")
    {
        run(root / "list");
    }

    SUBCASE("the open version after a closed one")
    {
        run(root / "open");
    }

    SUBCASE("physically removes the chunk of a live aggregate")
    {
        run(root / "physically");
    }

    SUBCASE("a fully deleted aggregate loses its file")
    {
        run(root / "physical");
    }

    SUBCASE("no name removes every fully deleted aggregate")
    {
        run(root / "all");
    }
}

}

}// namespace galimatron
