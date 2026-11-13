// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library/transforms/cpp2post_cpp/template_engine" * galimatron::test_axes::mark(galimatron::test_axes::translator_slice))
{

// gal2cpp, cpp2gal, then compilation
// the gal file keeps the written {{ }} text
// the compiled xml renders each version, including a tag version 1 inherits
//
TEST_CASE("a later version renders the inherited template")
{
    const auto root = path_to_galimatron_test_data / "translator_library/cpp2gal/template_engine";

    translate_slice(root, {
        .gal2cpp = true,
        .cpp2cpp_validate = false,
        .cpp2cpp_wizard = false,
        .cpp2xml = false,
        .xml2cpp = false,
        .cpp2gal = true,
        .cpp2post_cpp = true,
        .post_cpp2post_xml = true
    });

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_gal,
        .path_2 = root / test_folders::expected_gal
    }));
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_xml,
        .path_2 = root / test_folders::expected_xml
    }));
}

}

TEST_SUITE("galimatron/translator_library/template_engine_xslt" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

// a user xslt copies the language document and does not mention _te
// the gal keeps the written {{ }} text
// the language xml after that xslt still carries template-engine
// the compiled xml renders each version and omits the mark
//
TEST_CASE("a user xslt keeps the template mark")
{
    const auto root = path_to_galimatron_test_data / "translator_library/cpp2gal/template_engine_xslt";

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
}

}

}// namespace galimatron
