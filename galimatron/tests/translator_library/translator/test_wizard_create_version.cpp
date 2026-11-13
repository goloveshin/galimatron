// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_wizard_create_version")
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

    const auto root = path_to_galimatron_test_data / "translator_library/wizard_create_version";

    SUBCASE("root aggregate receives the next version")
    {
        run(root / "root_only");
    }

    SUBCASE("aggregates receive the next version, deleted when the previous declaration is deleted")
    {
        run(root / "aggregates");
    }

    SUBCASE("tags that use a dynamic label are copied into the next version")
    {
        run(root / "dynamic");
    }

    SUBCASE("a deleted dynamic tag stays deleted in the next version")
    {
        run(root / "deleted_tag");
    }

    SUBCASE("local stays on that aggregate and the body is copied")
    {
        run(root / "local_stays");
    }

    SUBCASE("create version once marks every aggregate")
    {
        run(root / "forced_once");
    }

    SUBCASE("create version local marks nested aggregates and the root once")
    {
        run(root / "forced_local");
    }

    SUBCASE("own stays and the body is not copied")
    {
        run(root / "own_stays");
    }

    SUBCASE("create version own marks nested aggregates and leaves the root unmarked")
    {
        run(root / "forced_own");
    }
}

}

}// namespace galimatron
