// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

namespace
{

using code_et = exception::details::validation_error_code_et;

void expect_ok(const path_t& root)
{
    translate_production(root);

    const auto filesystem = executable::filesystem::physically::make_filesystem();
    CHECK(filesystem.compare_directories({
        .path_1 = root / test_folders::actual_gal,
        .path_2 = root / test_folders::expected_gal
    }));
}

void expect_validation(
    const path_t& root,
    const code_et expected_code,
    const std::string& expected_what
)
{
    bool caught = false;

    print_leaf_exception(
        [&]
        {
            translate_production(root);
        },
        false,
        [&](
            const exception::domain::translator_library&,
            const exception::subdomain::translator_library::translator&,
            const exception::details::validation_error_code_et& code,
            const exception::e_what_info& info
        )
        {
            caught = true;
            CHECK(code == expected_code);
            CHECK(info.what == expected_what);
        }
    );

    CHECK(caught);
}

void expect_compiler(
    const path_t& root,
    const std::string& expected_what
)
{
    bool caught = false;

    print_leaf_exception(
        [&]
        {
            translate_production(root);
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

std::string incoming_path(const path_t& root, const std::string_view filename)
{
    return (root / test_folders::incoming_gal / filename).lexically_normal().string();
}

// .gal assigns aggregate indices in order, so a gap is built here
//
void expect_indices(
    const indices_t& indices,
    const std::optional<code_et>& expected_code,
    const std::string& expected_what
)
{
    translator::state::holder::language_part_t language;
    translator::state::builder::builder_t builder{ language };
    const auto chunk = builder.create_chunk({
        .root_node = builder.root_node(),
        .location = {}
    });
    builder.create_root_aggregate({
        .chunk_node = chunk,
        .name = "protocol",
        .location = {}
    });

    index_t name_index = 0;
    for (const auto index : indices)
    {
        builder.create_aggregate({
            .chunk_node = chunk,
            .name = std::format("item{}", name_index),
            .index = index,
            .location = {}
        });
        ++name_index;
    }

    translator::transforms::cpp2cpp_validate::validate_memo_t memo;
    translator::transforms::cpp2cpp_validate::cpp2cpp_validate_t validate(language, memo);

    if (!expected_code)
    {
        CHECK_NOTHROW(validate.is_aggregate_indices_unique_continuous());
        return;
    }

    bool caught = false;

    print_leaf_exception(
        [&]
        {
            validate.is_aggregate_indices_unique_continuous();
        },
        false,
        [&](
            const exception::domain::translator_library&,
            const exception::subdomain::translator_library::translator&,
            const exception::details::validation_error_code_et& code,
            const exception::e_what_info& info
        )
        {
            caught = true;
            CHECK(code == *expected_code);
            CHECK(info.what == expected_what);
        }
    );

    CHECK(caught);
}

}// namespace

TEST_SUITE("galimatron/translator_library/transforms/cpp2cpp_validate" * galimatron::test_axes::mark(galimatron::test_axes::product))
{

TEST_CASE("validate")
{
    const auto root = path_to_galimatron_test_data / "translator_library/validate";

    SUBCASE("one root aggregate")
    {
        expect_ok(root / "root_only_zero");
    }

    SUBCASE("no root aggregate")
    {
        expect_validation(
            root / "root_none",
            code_et::ROOT_AGGREGATE_MISSED,
            "root aggregate is absent"
        );
    }

    SUBCASE("two root aggregates")
    {
        const auto case_root = root / "root_two";
        const auto a = incoming_path(case_root, "a.gal");
        const auto b = incoming_path(case_root, "b.gal");
        const auto prefix = std::string("root aggregate is not single: files has root aggregate: ");

        bool caught = false;

        print_leaf_exception(
            [&]
            {
                translate_production(case_root);
            },
            false,
            [&](
                const exception::domain::translator_library&,
                const exception::subdomain::translator_library::translator&,
                const exception::details::validation_error_code_et& code,
                const exception::e_what_info& info
            )
            {
                caught = true;
                CHECK(code == code_et::ROOT_AGGREGATE_MUST_BE_ONE_ONLY);
                CHECK((info.what == prefix + a + "," + b || info.what == prefix + b + "," + a));
            }
        );

        CHECK(caught);
    }

    SUBCASE("version declarations 0, 1, 2")
    {
        expect_ok(root / "declarations_012");
    }

    SUBCASE("version declarations are empty")
    {
        expect_ok(root / "declarations_empty");
    }

    SUBCASE("version declarations start with 1")
    {
        expect_validation(
            root / "declarations_starts_with_1",
            code_et::ROOT_AGGREGATE_VERSION_DECLARATIONS_MUST_START_WITH_ZERO,
            "root aggregate version declarations do not start with 0: 1,2"
        );
    }

    SUBCASE("version declarations have a gap")
    {
        expect_validation(
            root / "declarations_gap",
            code_et::ROOT_AGGREGATE_VERSION_DECLARATIONS_MUST_BE_CONTINUOUS,
            "root aggregate version declarations are not continuous: 0,2"
        );
    }

    SUBCASE("version declarations are out of order")
    {
        expect_validation(
            root / "declarations_out_of_order",
            code_et::ROOT_AGGREGATE_VERSION_DECLARATIONS_MUST_BE_CONTINUOUS,
            "root aggregate version declarations are not continuous: 0,2,1"
        );
    }

    SUBCASE("version definitions repeat declarations")
    {
        expect_ok(root / "declarations_012");
    }

    SUBCASE("version definitions and declarations are empty")
    {
        expect_ok(root / "declarations_empty");
    }

    SUBCASE("version definition is missing")
    {
        expect_validation(
            root / "definitions_missing",
            code_et::ROOT_AGGREGATE_VERSION_DEFINITIONS_MUST_REPEAT_DECLARATIONS,
            "root aggregate version definitions do not repeat declarations: declarations 0,1, definitions 0"
        );
    }

    SUBCASE("version definition is extra")
    {
        expect_validation(
            root / "definitions_extra",
            code_et::ROOT_AGGREGATE_VERSION_DEFINITIONS_MUST_REPEAT_DECLARATIONS,
            "root aggregate version definitions do not repeat declarations: declarations 0, definitions 0,1"
        );
    }

    SUBCASE("the root version is local")
    {
        expect_validation(
            root / "root_version_local",
            code_et::ROOT_AGGREGATE_VERSION_CANNOT_BE_LOCAL,
            "root aggregate version 0 cannot be local"
        );
    }

    SUBCASE("the root version is own")
    {
        expect_validation(
            root / "root_version_own",
            code_et::ROOT_AGGREGATE_VERSION_CANNOT_BE_OWN,
            "root aggregate version 0 cannot be own"
        );
    }

    SUBCASE("version definitions are in another order")
    {
        expect_validation(
            root / "definitions_order",
            code_et::ROOT_AGGREGATE_VERSION_DEFINITIONS_MUST_REPEAT_DECLARATIONS,
            "root aggregate version definitions do not repeat declarations: declarations 0,1,2, definitions 0,2,1"
        );
    }

    SUBCASE("aggregate declarations match the root")
    {
        expect_ok(root / "aggregate_match");
    }

    SUBCASE("aggregate declarations are shorter than the root")
    {
        expect_validation(
            root / "aggregate_shorter",
            code_et::AGGREGATE_VERSION_DECLARATIONS_MUST_MATCH_ROOT,
            "aggregate 'item' version declarations do not match root aggregate: root 0,1,2, declarations 0,1"
        );
    }

    SUBCASE("deleted declarations run through the end")
    {
        expect_ok(root / "deleted_suffix");
    }

    SUBCASE("deleted declarations precede live ones")
    {
        expect_ok(root / "deleted_then_live");
    }

    SUBCASE("deleted declarations border live ones")
    {
        expect_ok(root / "deleted_gap");
    }

    SUBCASE("live declaration follows trailing deleted")
    {
        expect_validation(
            root / "deleted_resumed",
            code_et::AGGREGATE_VERSION_DECLARATIONS_DELETED_MUST_BE_CONTINUOUS,
            "aggregate 'item' version declarations: deleted status is not continuous: 0 deleted,1,2 deleted,3"
        );
    }

    SUBCASE("version definition is not deleted")
    {
        expect_ok(root / "deleted_suffix");
    }

    SUBCASE("version definition belongs to a deleted declaration")
    {
        expect_validation(
            root / "definitions_of_deleted",
            code_et::AGGREGATE_VERSION_DEFINITIONS_MUST_NOT_BE_DELETED,
            "aggregate 'item' version definition must not be deleted: 1"
        );
    }

    SUBCASE("version definitions repeat live declarations")
    {
        expect_ok(root / "deleted_suffix");
    }

    SUBCASE("live version definition is missing")
    {
        expect_validation(
            root / "definitions_missing_live",
            code_et::AGGREGATE_VERSION_DEFINITIONS_MUST_REPEAT_DECLARATIONS,
            "aggregate 'item' version definitions do not repeat declarations: declarations 0,1, definitions 0"
        );
    }

    SUBCASE("two wizard commands")
    {
        expect_validation(
            root / "wizard_two_commands",
            code_et::WIZARD_COMMAND_MUST_BE_ONE,
            "wizard command must be one: 2"
        );
    }

    SUBCASE("wizard create root aggregate in the aggregate file")
    {
        const auto case_root = root / "wizard_create_root_aggregate_command_file";
        expect_validation(
            case_root,
            code_et::WIZARD_CREATE_ROOT_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE,
            "wizard create root aggregate: cannot create the aggregate in the command file: " + incoming_path(case_root, "protocol.gal")
        );
    }

    SUBCASE("wizard create root aggregate when a root already exists")
    {
        const auto case_root = root / "wizard_root_exists";
        expect_validation(
            case_root,
            code_et::WIZARD_CREATE_ROOT_AGGREGATE_ROOT_AGGREGATE_MUST_BE_ABSENT,
            "wizard create root aggregate: root aggregate already exists: " + incoming_path(case_root, "a.gal")
        );
    }

    SUBCASE("wizard delete aggregate names an aggregate that does not exist")
    {
        expect_validation(
            root / "wizard_delete_aggregate_missing",
            code_et::WIZARD_DELETE_AGGREGATE_AGGREGATE_MUST_EXIST,
            "wizard delete aggregate: aggregate must exist: missing,gone"
        );
    }

    SUBCASE("wizard delete aggregate names the root aggregate")
    {
        expect_validation(
            root / "wizard_delete_aggregate_root",
            code_et::WIZARD_DELETE_AGGREGATE_CANNOT_DELETE_ROOT_AGGREGATE,
            "wizard delete aggregate: cannot delete the root aggregate: protocol"
        );
    }

    SUBCASE("wizard delete aggregate lists one name twice")
    {
        expect_validation(
            root / "wizard_delete_aggregate_names_repeated",
            code_et::WIZARD_DELETE_AGGREGATE_NAMES_MUST_BE_UNIQUE,
            "wizard delete aggregate: names must be unique: ping"
        );
    }

    SUBCASE("wizard delete aggregate when the last version is already deleted")
    {
        expect_validation(
            root / "wizard_delete_aggregate_already_deleted",
            code_et::WIZARD_DELETE_AGGREGATE_LAST_VERSION_IS_ALREADY_DELETED,
            "wizard delete aggregate: last version is already deleted: item"
        );
    }

    SUBCASE("wizard delete aggregate physically when a closed version still has the aggregate")
    {
        expect_validation(
            root / "wizard_delete_aggregate_physically_closed",
            code_et::WIZARD_DELETE_AGGREGATE_CANNOT_DELETE_CLOSED_VERSION,
            "wizard delete aggregate: cannot delete a closed version: item"
        );
    }

    SUBCASE("wizard copy aggregate names a missing source")
    {
        expect_validation(
            root / "wizard_copy_aggregate_missing",
            code_et::WIZARD_COPY_AGGREGATE_SOURCE_MUST_EXIST,
            "wizard copy aggregate: source must exist: missing"
        );
    }

    SUBCASE("wizard copy aggregate when the source is present in a closed version")
    {
        expect_validation(
            root / "wizard_copy_aggregate_closed",
            code_et::WIZARD_COPY_AGGREGATE_SOURCE_IN_CLOSED_VERSION,
            "wizard copy aggregate: source is present in a closed version: item"
        );
    }

    SUBCASE("wizard continue aggregate names a missing source")
    {
        expect_validation(
            root / "wizard_continue_aggregate_missing",
            code_et::WIZARD_CONTINUE_AGGREGATE_SOURCE_MUST_EXIST,
            "wizard continue aggregate: source must exist: missing"
        );
    }

    SUBCASE("wizard continue aggregate when the last version is already deleted")
    {
        expect_validation(
            root / "wizard_continue_aggregate_deleted",
            code_et::WIZARD_CONTINUE_AGGREGATE_LAST_VERSION_IS_ALREADY_DELETED,
            "wizard continue aggregate: last version is already deleted: ping"
        );
    }

    SUBCASE("wizard continue aggregate when the new name already exists")
    {
        expect_validation(
            root / "wizard_continue_aggregate_present",
            code_et::WIZARD_CONTINUE_AGGREGATE_AGGREGATE_MUST_BE_ABSENT,
            "wizard continue aggregate: aggregate must be absent: pong"
        );
    }

    SUBCASE("wizard copy aggregate when the new name already exists")
    {
        expect_validation(
            root / "wizard_copy_aggregate_present",
            code_et::WIZARD_COPY_AGGREGATE_AGGREGATE_MUST_BE_ABSENT,
            "wizard copy aggregate: aggregate must be absent: pong"
        );
    }

    SUBCASE("wizard delete aggregate physically when ordering names the aggregate")
    {
        expect_validation(
            root / "wizard_delete_aggregate_physically_ordered",
            code_et::WIZARD_DELETE_AGGREGATE_ORDERING_NAMES_AGGREGATE,
            "wizard delete aggregate: ordering names the aggregate: item"
        );
    }

    SUBCASE("wizard delete aggregate when the last version is closed")
    {
        expect_validation(
            root / "wizard_delete_aggregate_closed",
            code_et::WIZARD_DELETE_AGGREGATE_CANNOT_DELETE_CLOSED_VERSION,
            "wizard delete aggregate: cannot delete a closed version: item"
        );
    }

    SUBCASE("wizard delete version when the last version is closed")
    {
        expect_validation(
            root / "wizard_delete_version_closed",
            code_et::WIZARD_DELETE_VERSION_CANNOT_DELETE_CLOSED_VERSION,
            "wizard delete version: cannot delete a closed version: 0"
        );
    }

    SUBCASE("wizard delete version lists a closed version")
    {
        expect_validation(
            root / "wizard_delete_version_closed_listed",
            code_et::WIZARD_DELETE_VERSION_CANNOT_DELETE_CLOSED_VERSION,
            "wizard delete version: cannot delete a closed version: 0"
        );
    }

    SUBCASE("wizard delete version for versions that do not exist")
    {
        expect_validation(
            root / "wizard_delete_version_missing",
            code_et::WIZARD_DELETE_VERSION_VERSION_MUST_EXIST,
            "wizard delete version: version must exist: 8,9"
        );
    }

    SUBCASE("wizard delete version leaves a gap")
    {
        expect_validation(
            root / "wizard_delete_version_gap",
            code_et::WIZARD_DELETE_VERSION_MUST_STAY_CONTINUOUS,
            "wizard delete version: versions must stay continuous: 0,2"
        );
    }

    SUBCASE("wizard collapse version for versions that do not exist")
    {
        expect_validation(
            root / "wizard_collapse_version_missing",
            code_et::WIZARD_COLLAPSE_VERSION_VERSION_MUST_EXIST,
            "wizard collapse version: version must exist: 8,9"
        );
    }

    SUBCASE("wizard collapse version range descends")
    {
        expect_validation(
            root / "wizard_collapse_version_range",
            code_et::WIZARD_COLLAPSE_VERSION_RANGE_MUST_ASCEND,
            "wizard collapse version: version range must ascend: 4-1"
        );
    }

    SUBCASE("anonymous version has no owner")
    {
        expect_validation(
            root / "anonymous_without_owner",
            code_et::ANONYMOUS_VERSION_MUST_HAVE_OWNER,
            "aggregate 'protocol' version before 1 must have an owner"
        );
    }

    SUBCASE("version before names a version that is not the next body")
    {
        expect_validation(
            root / "anonymous_not_before",
            code_et::ANONYMOUS_VERSION_MUST_PRECEDE_OWNER,
            "aggregate 'protocol' version before 2 must be immediately before that version"
        );
    }

    SUBCASE("anonymous version stays when its owner is deleted")
    {
        expect_validation(
            root / "anonymous_owner_deleted",
            code_et::ANONYMOUS_VERSION_MUST_BE_DELETED,
            "aggregate 'item' anonymous version must be deleted: owner 1"
        );
    }

    SUBCASE("wizard create tag in a chunk without an aggregate name")
    {
        expect_validation(
            root / "wizard_create_tag_not_named",
            code_et::WIZARD_VERSION_STATEMENT_AGGREGATE_MUST_BE_NAMED,
            "wizard create tag: aggregate must be named"
        );
    }

    SUBCASE("wizard create tag for an aggregate that does not exist")
    {
        expect_validation(
            root / "wizard_create_tag_missing",
            code_et::WIZARD_VERSION_STATEMENT_AGGREGATE_MUST_EXIST,
            "wizard create tag: aggregate must exist: missing"
        );
    }

    SUBCASE("wizard create tag when the last version is closed")
    {
        expect_validation(
            root / "wizard_create_tag_closed",
            code_et::WIZARD_VERSION_STATEMENT_VERSION_MUST_BE_OPEN,
            "wizard create tag: version must be the open version: protocol"
        );
    }

    SUBCASE("wizard create tag when the last version is deleted")
    {
        expect_validation(
            root / "wizard_create_tag_deleted",
            code_et::WIZARD_VERSION_STATEMENT_VERSION_IS_DELETED,
            "wizard create tag: version is deleted: item"
        );
    }

    SUBCASE("wizard create ordering for an aggregate that is not the root")
    {
        expect_validation(
            root / "wizard_create_ordering_not_root",
            code_et::WIZARD_VERSION_STATEMENT_AGGREGATE_MUST_BE_ROOT,
            "wizard create ordering: aggregate must be the root aggregate: item"
        );
    }

    SUBCASE("wizard batch and another wizard command")
    {
        expect_validation(
            root / "wizard_batch_and_command",
            code_et::WIZARD_COMMAND_MUST_BE_ONE,
            "wizard command must be one: 2"
        );
    }

    SUBCASE("wizard batch delete hash")
    {
        expect_validation(
            root / "wizard_batch_delete_hash",
            code_et::WIZARD_BATCH_COMMAND_MUST_BE_CREATE,
            "wizard batch executes create commands, delete tag, delete version, and collapse version only: wizard delete hash"
        );
    }

    SUBCASE("wizard create hash for a version that does not exist")
    {
        expect_validation(
            root / "wizard_create_hash_version_missing",
            code_et::WIZARD_CREATE_HASH_VERSION_MUST_EXIST,
            "wizard create hash: version must exist: 9"
        );
    }

    SUBCASE("wizard delete hash for versions that do not exist")
    {
        expect_validation(
            root / "wizard_delete_hash_version_missing",
            code_et::WIZARD_DELETE_HASH_VERSION_MUST_EXIST,
            "wizard delete hash: version must exist: 8,9"
        );
    }

    SUBCASE("wizard create hash of the last version when none exists")
    {
        expect_validation(
            root / "wizard_create_hash_last_missing",
            code_et::WIZARD_CREATE_HASH_VERSION_MUST_EXIST,
            "wizard create hash: last version must exist"
        );
    }

    SUBCASE("wizard delete hash of the last version when none exists")
    {
        expect_validation(
            root / "wizard_delete_hash_last_missing",
            code_et::WIZARD_DELETE_HASH_VERSION_MUST_EXIST,
            "wizard delete hash: last version must exist"
        );
    }

    SUBCASE("tag uses a label that is present")
    {
        expect_ok(root / "label_present");
    }

    SUBCASE("tag uses a label that is absent")
    {
        expect_compiler(
            root / "label_missing",
            "no label 'field' for tag: " + incoming_path(root / "label_missing", "protocol.gal") + " version 0"
        );
    }

    SUBCASE("tag uses a label declared later")
    {
        expect_compiler(
            root / "label_later",
            "no label 'field' for tag: " + incoming_path(root / "label_later", "protocol.gal") + " version 0"
        );
    }

    SUBCASE("delete of a label")
    {
        expect_compiler(
            root / "delete_label",
            "cannot delete label 'field': " + incoming_path(root / "delete_label", "protocol.gal") + " version 0"
        );
    }

    SUBCASE("hashes match")
    {
        expect_ok(root / "hash_match");
    }

    SUBCASE("create hash force still checks a version it does not name")
    {
        expect_validation(
            root / "hash_force_other",
            code_et::HASH_DOES_NOT_MATCH,
            "hash does not match: version 1: protocol"
        );
    }

    SUBCASE("hashes do not match")
    {
        expect_validation(
            root / "hash_mismatch",
            code_et::HASH_DOES_NOT_MATCH,
            "hash does not match: version 1: item; version 0: protocol"
        );
    }

    SUBCASE("aggregate names are unique")
    {
        expect_ok(root / "aggregate_match");
    }

    SUBCASE("aggregate names are repeated")
    {
        expect_validation(
            root / "names_duplicate",
            code_et::AGGREGATE_NAMES_MUST_BE_UNIQUE,
            "aggregate names are not unique: item"
        );
    }

    SUBCASE("a version hash is on every aggregate")
    {
        expect_ok(root / "hash_match");
    }

    SUBCASE("a version hash is absent on an aggregate")
    {
        expect_validation(
            root / "hash_absent",
            code_et::HASH_MUST_BE_ON_EVERY_AGGREGATE,
            "hash is absent: item version 0"
        );
    }

    SUBCASE("wizard create aggregate when the last version is closed")
    {
        expect_validation(
            root / "wizard_create_aggregate_closed",
            code_et::WIZARD_CREATE_AGGREGATE_LAST_VERSION_MUST_BE_OPEN,
            "wizard create aggregate: cannot create an aggregate when the last version is closed"
        );
    }

    SUBCASE("wizard create aggregate in the aggregate file")
    {
        const auto case_root = root / "wizard_create_aggregate_command_file";
        expect_validation(
            case_root,
            code_et::WIZARD_CREATE_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE,
            "wizard create aggregate: cannot create the aggregate in the command file: " + incoming_path(case_root, "item.gal")
        );
    }

    SUBCASE("wizard create aggregate lists one name twice")
    {
        expect_validation(
            root / "wizard_create_aggregate_names_repeated",
            code_et::WIZARD_CREATE_AGGREGATE_NAMES_MUST_BE_UNIQUE,
            "wizard create aggregate: names must be unique: ping"
        );
    }

    SUBCASE("wizard create aggregate lists the command file among other names")
    {
        const auto case_root = root / "wizard_create_aggregate_command_file_list";
        expect_validation(
            case_root,
            code_et::WIZARD_CREATE_AGGREGATE_COMMAND_FILE_IS_THE_AGGREGATE,
            "wizard create aggregate: cannot create the aggregate in the command file: " + incoming_path(case_root, "item.gal")
        );
    }

    SUBCASE("wizard create aggregate when listed chunk files already exist")
    {
        const auto case_root = root / "wizard_create_aggregate_list_chunk_exists";
        expect_validation(
            case_root,
            code_et::WIZARD_CREATE_AGGREGATE_CHUNK_MUST_BE_ABSENT,
            "wizard create aggregate: chunk already exists: "
                + incoming_path(case_root, "ping.gal")
                + ","
                + incoming_path(case_root, "pong.gal")
        );
    }

    SUBCASE("wizard create aggregate when the chunk file already exists")
    {
        const auto case_root = root / "wizard_create_aggregate_chunk_exists";
        expect_validation(
            case_root,
            code_et::WIZARD_CREATE_AGGREGATE_CHUNK_MUST_BE_ABSENT,
            "wizard create aggregate: chunk already exists: " + incoming_path(case_root, "item.gal")
        );
    }
}

}

TEST_SUITE("galimatron/translator_library/transforms/cpp2cpp_validate" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("aggregate indices")
{
    SUBCASE("indices are 0, 1")
    {
        expect_indices({ 0, 1 }, {}, {});
    }

    SUBCASE("indices have a gap")
    {
        expect_indices(
            { 0, 2 },
            code_et::AGGREGATE_INDICES_MUST_BE_UNIQUE_CONTINUOUS,
            "aggregate indices are not unique and continuous from 0: 0,2"
        );
    }

    SUBCASE("indices are repeated")
    {
        expect_indices(
            { 0, 0 },
            code_et::AGGREGATE_INDICES_MUST_BE_UNIQUE_CONTINUOUS,
            "aggregate indices are not unique and continuous from 0: 0,0"
        );
    }
}

}

}// namespace galimatron
