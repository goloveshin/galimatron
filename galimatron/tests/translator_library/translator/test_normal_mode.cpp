// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

// input and output are the same folder. item.gal is read, then the wizard removes that aggregate,
// so the run does not rewrite the file and deletes it. source_gal is staged again on the next run.
// the xml of a normal run keeps the absolute file paths, so it is not an oracle
//
TEST_CASE("normal mode deletes a gal file that was not rewritten")
{
    const auto root = path_to_galimatron_test_data / "translator_library/normal_mode";
    stage_directory(
        root / "source_gal",
        root / test_folders::incoming_gal
    );

    translate_production(root);

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::incoming_gal,
        .path_2 = root / test_folders::expected_gal
    }));
}

}

}// namespace galimatron
