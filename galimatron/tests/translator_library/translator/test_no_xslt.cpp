// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("test_no_xslt")
{
    // no user xslt. the log is an oracle, so Saxon is absent from it
    const auto root = path_to_galimatron_test_data / "translator_library/no_xslt";

    translate_production(root);

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_log,
        .path_2 = root / test_folders::expected_log
    }));
}

}

}// namespace galimatron
