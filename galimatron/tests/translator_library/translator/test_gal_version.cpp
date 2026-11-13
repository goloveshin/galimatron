// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_gal_version")
{
    const auto root = path_to_galimatron_test_data / "translator_library/gal_version";

    SUBCASE("version 1")
    {
        const auto case_root = root / "version_1";
        translate_production(case_root);

        const auto filesystem = executable::filesystem::physically::make_filesystem();
        CHECK(filesystem.compare_directories({
            .path_1 = case_root / test_folders::actual_gal,
            .path_2 = case_root / test_folders::expected_gal
        }));
    }

    SUBCASE("unknown version")
    {
        bool caught = false;

        print_leaf_exception(
            [&]
            {
                translate_production(root / "version_2");
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
