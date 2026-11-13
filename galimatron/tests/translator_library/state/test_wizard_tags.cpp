// galimatron
#include "galimatron/translator_library/state/wizard/tags.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

using namespace galimatron::translator;

TEST_SUITE("galimatron/translator_library/state/wizard/tags" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("wizard tags")
{
    using namespace state::wizard::tags;

    auto test = [&](
        const auto& actual,
        const auto& expected,
        const auto& expected_base64
    )
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    SUBCASE("command")
    {
        test(command::must_be_one, "wizard command must be one", "d2l6YXJkIGNvbW1hbmQgbXVzdCBiZSBvbmU="_base64);
    }

    SUBCASE("create root aggregate")
    {
        test(create_root_aggregate::command, "wizard create root aggregate", "d2l6YXJkIGNyZWF0ZSByb290IGFnZ3JlZ2F0ZQ=="_base64);
        test(create_root_aggregate::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(create_root_aggregate::command_file_is_the_aggregate, "cannot create the aggregate in the command file", "Y2Fubm90IGNyZWF0ZSB0aGUgYWdncmVnYXRlIGluIHRoZSBjb21tYW5kIGZpbGU="_base64);
        test(create_root_aggregate::chunk_already_exists, "chunk already exists", "Y2h1bmsgYWxyZWFkeSBleGlzdHM="_base64);
        test(create_root_aggregate::root_aggregate_already_exists, "root aggregate already exists", "cm9vdCBhZ2dyZWdhdGUgYWxyZWFkeSBleGlzdHM="_base64);
    }

    SUBCASE("create version")
    {
        test(create_version::command, "wizard create version", "d2l6YXJkIGNyZWF0ZSB2ZXJzaW9u"_base64);
        test(create_version::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(create_version::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
    }

    SUBCASE("delete version")
    {
        test(delete_version::command, "wizard delete version", "d2l6YXJkIGRlbGV0ZSB2ZXJzaW9u"_base64);
        test(delete_version::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(delete_version::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
        test(delete_version::last_version_must_exist, "last version must exist", "bGFzdCB2ZXJzaW9uIG11c3QgZXhpc3Q="_base64);
        test(delete_version::version_must_exist, "version must exist", "dmVyc2lvbiBtdXN0IGV4aXN0"_base64);
        test(delete_version::cannot_delete_closed_version, "cannot delete a closed version", "Y2Fubm90IGRlbGV0ZSBhIGNsb3NlZCB2ZXJzaW9u"_base64);
        test(delete_version::versions_must_stay_continuous, "versions must stay continuous", "dmVyc2lvbnMgbXVzdCBzdGF5IGNvbnRpbnVvdXM="_base64);
    }

    SUBCASE("collapse version")
    {
        test(collapse_version::command, "wizard collapse version", "d2l6YXJkIGNvbGxhcHNlIHZlcnNpb24="_base64);
        test(collapse_version::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(collapse_version::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
        test(collapse_version::version_must_exist, "version must exist", "dmVyc2lvbiBtdXN0IGV4aXN0"_base64);
        test(collapse_version::owner_must_be_present, "owner version must be present", "b3duZXIgdmVyc2lvbiBtdXN0IGJlIHByZXNlbnQ="_base64);
        test(collapse_version::range_must_ascend, "version range must ascend", "dmVyc2lvbiByYW5nZSBtdXN0IGFzY2VuZA=="_base64);
    }

    SUBCASE("create hash")
    {
        test(create_hash::command, "wizard create hash", "d2l6YXJkIGNyZWF0ZSBoYXNo"_base64);
        test(create_hash::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(create_hash::version_must_exist, "version must exist", "dmVyc2lvbiBtdXN0IGV4aXN0"_base64);
    }

    SUBCASE("delete hash")
    {
        test(delete_hash::command, "wizard delete hash", "d2l6YXJkIGRlbGV0ZSBoYXNo"_base64);
        test(delete_hash::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(delete_hash::version_must_exist, "version must exist", "dmVyc2lvbiBtdXN0IGV4aXN0"_base64);
    }

    SUBCASE("create aggregate")
    {
        test(create_aggregate::command, "wizard create aggregate", "d2l6YXJkIGNyZWF0ZSBhZ2dyZWdhdGU="_base64);
        test(create_aggregate::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(create_aggregate::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
        test(create_aggregate::names_must_be_unique, "names must be unique", "bmFtZXMgbXVzdCBiZSB1bmlxdWU="_base64);
        test(create_aggregate::command_file_is_the_aggregate, "cannot create the aggregate in the command file", "Y2Fubm90IGNyZWF0ZSB0aGUgYWdncmVnYXRlIGluIHRoZSBjb21tYW5kIGZpbGU="_base64);
        test(create_aggregate::chunk_already_exists, "chunk already exists", "Y2h1bmsgYWxyZWFkeSBleGlzdHM="_base64);
        test(create_aggregate::last_version_must_be_open, "cannot create an aggregate when the last version is closed", "Y2Fubm90IGNyZWF0ZSBhbiBhZ2dyZWdhdGUgd2hlbiB0aGUgbGFzdCB2ZXJzaW9uIGlzIGNsb3NlZA=="_base64);
    }

    SUBCASE("copy aggregate")
    {
        test(copy_aggregate::command, "wizard copy aggregate", "d2l6YXJkIGNvcHkgYWdncmVnYXRl"_base64);
        test(copy_aggregate::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(copy_aggregate::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
        test(copy_aggregate::source_must_exist, "source must exist", "c291cmNlIG11c3QgZXhpc3Q="_base64);
        test(copy_aggregate::cannot_copy_root_aggregate, "cannot copy the root aggregate", "Y2Fubm90IGNvcHkgdGhlIHJvb3QgYWdncmVnYXRl"_base64);
        test(copy_aggregate::names_must_be_unique, "names must be unique", "bmFtZXMgbXVzdCBiZSB1bmlxdWU="_base64);
        test(copy_aggregate::aggregate_must_be_absent, "aggregate must be absent", "YWdncmVnYXRlIG11c3QgYmUgYWJzZW50"_base64);
        test(copy_aggregate::command_file_is_the_aggregate, "cannot copy the aggregate onto the command file", "Y2Fubm90IGNvcHkgdGhlIGFnZ3JlZ2F0ZSBvbnRvIHRoZSBjb21tYW5kIGZpbGU="_base64);
        test(copy_aggregate::chunk_already_exists, "chunk already exists", "Y2h1bmsgYWxyZWFkeSBleGlzdHM="_base64);
        test(copy_aggregate::source_in_closed_version, "source is present in a closed version", "c291cmNlIGlzIHByZXNlbnQgaW4gYSBjbG9zZWQgdmVyc2lvbg=="_base64);
    }

    SUBCASE("continue aggregate")
    {
        test(continue_aggregate::command, "wizard continue aggregate", "d2l6YXJkIGNvbnRpbnVlIGFnZ3JlZ2F0ZQ=="_base64);
        test(continue_aggregate::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(continue_aggregate::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
        test(continue_aggregate::source_must_exist, "source must exist", "c291cmNlIG11c3QgZXhpc3Q="_base64);
        test(continue_aggregate::cannot_continue_root_aggregate, "cannot continue the root aggregate", "Y2Fubm90IGNvbnRpbnVlIHRoZSByb290IGFnZ3JlZ2F0ZQ=="_base64);
        test(continue_aggregate::names_must_be_unique, "names must be unique", "bmFtZXMgbXVzdCBiZSB1bmlxdWU="_base64);
        test(continue_aggregate::aggregate_must_be_absent, "aggregate must be absent", "YWdncmVnYXRlIG11c3QgYmUgYWJzZW50"_base64);
        test(continue_aggregate::command_file_is_the_aggregate, "cannot continue the aggregate onto the command file", "Y2Fubm90IGNvbnRpbnVlIHRoZSBhZ2dyZWdhdGUgb250byB0aGUgY29tbWFuZCBmaWxl"_base64);
        test(continue_aggregate::chunk_already_exists, "chunk already exists", "Y2h1bmsgYWxyZWFkeSBleGlzdHM="_base64);
        test(continue_aggregate::last_version_must_exist, "last version must exist", "bGFzdCB2ZXJzaW9uIG11c3QgZXhpc3Q="_base64);
        test(continue_aggregate::last_version_is_already_deleted, "last version is already deleted", "bGFzdCB2ZXJzaW9uIGlzIGFscmVhZHkgZGVsZXRlZA=="_base64);
    }

    SUBCASE("version statement")
    {
        test(version_statement::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(version_statement::aggregate_must_be_named, "aggregate must be named", "YWdncmVnYXRlIG11c3QgYmUgbmFtZWQ="_base64);
        test(version_statement::aggregate_must_be_root, "aggregate must be the root aggregate", "YWdncmVnYXRlIG11c3QgYmUgdGhlIHJvb3QgYWdncmVnYXRl"_base64);
        test(version_statement::version_must_be_open, "version must be the open version", "dmVyc2lvbiBtdXN0IGJlIHRoZSBvcGVuIHZlcnNpb24="_base64);
        test(version_statement::version_is_deleted, "version is deleted", "dmVyc2lvbiBpcyBkZWxldGVk"_base64);
    }

    SUBCASE("create label")
    {
        test(create_label::command, "wizard create label", "d2l6YXJkIGNyZWF0ZSBsYWJlbA=="_base64);
    }

    SUBCASE("create tag")
    {
        test(create_tag::command, "wizard create tag", "d2l6YXJkIGNyZWF0ZSB0YWc="_base64);
    }

    SUBCASE("delete tag")
    {
        test(delete_tag::command, "wizard delete tag", "d2l6YXJkIGRlbGV0ZSB0YWc="_base64);
    }

    SUBCASE("create ordering")
    {
        test(create_ordering::command, "wizard create ordering", "d2l6YXJkIGNyZWF0ZSBvcmRlcmluZw=="_base64);
    }

    SUBCASE("delete aggregate")
    {
        test(delete_aggregate::command, "wizard delete aggregate", "d2l6YXJkIGRlbGV0ZSBhZ2dyZWdhdGU="_base64);
        test(delete_aggregate::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(delete_aggregate::root_aggregate_must_exist, "root aggregate must exist", "cm9vdCBhZ2dyZWdhdGUgbXVzdCBleGlzdA=="_base64);
        test(delete_aggregate::names_must_be_unique, "names must be unique", "bmFtZXMgbXVzdCBiZSB1bmlxdWU="_base64);
        test(delete_aggregate::aggregate_must_exist, "aggregate must exist", "YWdncmVnYXRlIG11c3QgZXhpc3Q="_base64);
        test(delete_aggregate::cannot_delete_root_aggregate, "cannot delete the root aggregate", "Y2Fubm90IGRlbGV0ZSB0aGUgcm9vdCBhZ2dyZWdhdGU="_base64);
        test(delete_aggregate::last_version_must_exist, "last version must exist", "bGFzdCB2ZXJzaW9uIG11c3QgZXhpc3Q="_base64);
        test(delete_aggregate::last_version_is_already_deleted, "last version is already deleted", "bGFzdCB2ZXJzaW9uIGlzIGFscmVhZHkgZGVsZXRlZA=="_base64);
        test(delete_aggregate::cannot_delete_closed_version, "cannot delete a closed version", "Y2Fubm90IGRlbGV0ZSBhIGNsb3NlZCB2ZXJzaW9u"_base64);
        test(delete_aggregate::version_definition_must_exist, "version definition must exist", "dmVyc2lvbiBkZWZpbml0aW9uIG11c3QgZXhpc3Q="_base64);
        test(delete_aggregate::ordering_names_the_aggregate, "ordering names the aggregate", "b3JkZXJpbmcgbmFtZXMgdGhlIGFnZ3JlZ2F0ZQ=="_base64);
    }

    SUBCASE("batch")
    {
        test(batch::command, "wizard batch", "d2l6YXJkIGJhdGNo"_base64);
        test(batch::context_must_be_chunk, "may be executed in chunk context only", "bWF5IGJlIGV4ZWN1dGVkIGluIGNodW5rIGNvbnRleHQgb25seQ=="_base64);
        test(batch::command_must_be_allowed, "executes create commands, delete tag, delete version, and collapse version only", "ZXhlY3V0ZXMgY3JlYXRlIGNvbW1hbmRzLCBkZWxldGUgdGFnLCBkZWxldGUgdmVyc2lvbiwgYW5kIGNvbGxhcHNlIHZlcnNpb24gb25seQ=="_base64);
    }
}

}
