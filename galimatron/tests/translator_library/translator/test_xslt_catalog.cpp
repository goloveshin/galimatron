// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

namespace
{

// both cases read bin/../share from this one process.
// configure can write that folder only once, so it cannot hide the catalog from the first case and show it to the second.
// the first case deletes share before the run. the second case copies the catalog and gal-functions.xsl back before its run
//
path_t installed_share_directory()
{
    // the test executable is in the same bin directory as galimatron_test_data
    return (path_to_galimatron_test_data.parent_path() / ".." / "share").lexically_normal();
}

// removes share so the next run logs that the catalog did not resolve
//
void delete_installed_catalog()
{
    std::error_code error;
    fs::remove_all(installed_share_directory(), error);
    CHECK_FALSE(error);
}

// copies catalog.xml and gal-functions.xsl into share/galimatron
//
void copy_installed_catalog()
{
    const auto root = installed_share_directory() / "galimatron";
    stage_file(path_to_xslt_catalog, root / "catalog.xml");
    stage_file(path_to_gal_functions_xsl, root / "xslt" / "gal-functions.xsl");
}

}// namespace

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

// the file log is debug. share is gone, so the catalog does not resolve.
// filesystem and config debug stay out of this log: those lines carry absolute paths
//
TEST_CASE("a missing catalog does not resolve")
{
    delete_installed_catalog();

    const auto root = path_to_galimatron_test_data / "translator_library/xslt_catalog";

    translate_production(root);

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_log,
        .path_2 = root / test_folders::expected_log
    }));
}

// the file log is debug. the copied catalog resolves the installed include.
// the log names that relative path. filesystem and config debug stay out of this log
//
TEST_CASE("an installed catalog resolves the xslt library")
{
    copy_installed_catalog();

    const auto root = path_to_galimatron_test_data / "translator_library/xslt_catalog_resolved";

    translate_production(root);

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_log,
        .path_2 = root / test_folders::expected_log
    }));
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_gal,
        .path_2 = root / test_folders::expected_gal
    }));
}

}

}// namespace galimatron
