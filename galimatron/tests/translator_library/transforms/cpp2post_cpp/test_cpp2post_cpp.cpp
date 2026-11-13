// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

namespace
{

// gal2cpp, cpp2post_cpp, then the compiled xml
//
void run(const path_t& root)
{
    translate_slice(root, {
        .gal2cpp = true,
        .cpp2cpp_validate = false,
        .cpp2cpp_wizard = false,
        .cpp2xml = false,
        .xml2cpp = false,
        .cpp2gal = false,
        .cpp2post_cpp = true,
        .post_cpp2post_xml = true
    });
}

void expect_compiled(const path_t& root)
{
    run(root);

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_xml,
        .path_2 = root / test_folders::expected_xml
    }));
}

// the compiler throw: file, aggregate, version, and the message
//
void expect_compiler(
    const path_t& root,
    const std::string& expected_what
)
{
    bool caught = false;

    print_leaf_exception(
        [&]
        {
            run(root);
        },
        false,
        [&](
            const exception::domain::translator_library&,
            const exception::subdomain::translator_library::cpp2post_cpp&,
            const exception::e_what_info& info,
            const exception::e_path_info& path,
            const exception::e_aggregate_info& aggregate,
            const exception::e_version_info& version
        )
        {
            caught = true;
            CHECK(info.what == expected_what);
            REQUIRE(path.path);
            CHECK(path.path->lexically_normal() == (root / test_folders::incoming_gal / "protocol.gal").lexically_normal());
            CHECK(aggregate.name == "protocol");
            CHECK(version.version == 0);
        }
    );

    CHECK(caught);
}

}// namespace

TEST_SUITE("galimatron/translator_library" * galimatron::test_axes::mark(galimatron::test_axes::translator_slice))
{

// compiled tags, a deleted version, ordering, label constraints, and unnamed-tag names
//
TEST_CASE("test_cpp2post_cpp")
{
    const auto root = path_to_galimatron_test_data / "translator_library/cpp2post_cpp";

    SUBCASE("delete speed in the same version")
    {
        expect_compiled(root / "delete_tag/same_version");
    }

    SUBCASE("delete speed from the next version")
    {
        expect_compiled(root / "delete_tag/later_version");
    }

    SUBCASE("delete speed, then declare it again")
    {
        expect_compiled(root / "delete_tag/redeclare");
    }

    SUBCASE("delete a tag that was never declared")
    {
        expect_compiled(root / "delete_tag/absent");
    }

    SUBCASE("delete; drops the rest of the version")
    {
        expect_compiled(root / "delete_version/strip");
    }

    SUBCASE("the later ordering of the same name is kept")
    {
        expect_compiled(root / "ordering/later_wins");
    }

    SUBCASE("an ordering names an unknown aggregate")
    {
        expect_compiler(
            root / "ordering/unknown",
            "ordering 'precedes' names 'typo', which is not an aggregate"
        );
    }

    SUBCASE("an ordering where an aggregate precedes itself")
    {
        expect_compiler(
            root / "ordering/twice",
            "ordering 'precedes' is not a strict weak order: 'item' precedes itself"
        );
    }

    SUBCASE("an ordering where two aggregates precede each other")
    {
        expect_compiler(
            root / "ordering/cycle",
            "ordering 'precedes' is not a strict weak order: 'item' and 'live' precede each other"
        );
    }

    SUBCASE("an ordering that is not a strict weak order")
    {
        expect_compiler(
            root / "ordering/not_weak",
            "ordering 'precedes' is not a strict weak order: 'color' does not precede 'live'"
        );
    }

    SUBCASE("aggregates tied by the ordering keep the default order")
    {
        expect_compiled(root / "ordering/tie");
    }

    SUBCASE("parts join into one chain")
    {
        expect_compiled(root / "ordering/chain");
    }

    SUBCASE("a diamond keeps the tied aggregates in the default order")
    {
        expect_compiled(root / "ordering/diamond");
    }

    SUBCASE("a short side branch is not a strict weak order")
    {
        expect_compiler(
            root / "ordering/fork",
            "ordering 'precedes' is not a strict weak order: 'pong' does not precede 'color'"
        );
    }

    SUBCASE("an ordering names an aggregate absent from the version")
    {
        expect_compiler(
            root / "ordering/absent",
            "ordering 'precedes' names 'item', which is absent from version 0"
        );
    }

    SUBCASE("an aggregate the ordering does not name stays after the named ones")
    {
        expect_compiled(root / "ordering/partial");
    }

    SUBCASE("an unnamed tag takes its name from the label")
    {
        expect_compiled(root / "naming/from_label");
    }

    SUBCASE("unnamed tag with no naming label")
    {
        expect_compiler(
            root / "naming/no_namer",
            "tag '9' has no name and has no label with naming"
        );
    }

    SUBCASE("tag-name-from is not one of the label values")
    {
        expect_compiler(
            root / "naming/namer_not_in_values",
            "label 'field' has tag-name-from but it is not among its values"
        );
    }

    SUBCASE("no tag value at the tag-name-from index")
    {
        expect_compiler(
            root / "naming/no_value",
            "tag '9' has no value with index 1 for tag's name"
        );
    }

    SUBCASE("tag-name-from value is not an identifier")
    {
        expect_compiler(
            root / "naming/not_identifier",
            "tag '9' has value not-a-name with index 1 for tag's name but it is not a identifier"
        );
    }

    SUBCASE("two labels name the tag differently")
    {
        expect_compiler(
            root / "naming/different_names",
            "labels 'a' and 'b' name the tag differently: speed and other"
        );
    }

    SUBCASE("a static label repeats a value")
    {
        expect_compiler(
            root / "label/duplicate_value",
            "label 'field' has duplicate value 'type'"
        );
    }

    SUBCASE("a label statement is named important")
    {
        expect_compiler(
            root / "label/important",
            "label name 'important' is the built-in tag marker"
        );
    }

    SUBCASE("once reaches nested aggregates of this version and local stays here")
    {
        expect_compiled(root / "tag_mode");
    }

    SUBCASE("once and local versions")
    {
        expect_compiled(root / "version_mode");
    }

    SUBCASE("an own version keeps this aggregate and drops the root")
    {
        expect_compiled(root / "version_own");
    }
}

}

}// namespace galimatron
