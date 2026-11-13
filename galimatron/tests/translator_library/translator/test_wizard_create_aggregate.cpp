// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_wizard_create_aggregate")
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

    const auto root = path_to_galimatron_test_data / "translator_library/wizard_create_aggregate";

    SUBCASE("command file stays without the aggregate")
    {
        run(root / "in_empty_chunk");
    }

    SUBCASE("chunk that already has a root aggregate gets a new file")
    {
        run(root / "in_root_chunk");
    }

    SUBCASE("a list of names writes one file each")
    {
        run(root / "list");
    }

    SUBCASE("in names the directory under the command file")
    {
        run(root / "in_directory");
    }
}

}

}// namespace galimatron
