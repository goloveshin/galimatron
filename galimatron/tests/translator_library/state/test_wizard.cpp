// galimatron
#include "galimatron/translator_library/state/version_status.hpp"
#include "galimatron/translator_library/state/wizard/forward.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

using namespace galimatron::translator;

TEST_SUITE("galimatron/translator_library/state/wizard" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("to_string")
{
    auto test = [&](
        const auto& actual,
        const auto& expected,
        const auto& expected_base64
    )
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    SUBCASE("wizard context")
    {
        using enum state::wizard::context_et;

        test(to_string(CHUNK), "chunk", "Y2h1bms="_base64);
        test(to_string(ROOT_AGGREGATE), "root-aggregate", "cm9vdC1hZ2dyZWdhdGU="_base64);
        test(to_string(ROOT_AGGREGATE_VERSION_DECLARATIONS), "root-aggregate-version-declarations", "cm9vdC1hZ2dyZWdhdGUtdmVyc2lvbi1kZWNsYXJhdGlvbnM="_base64);
        test(to_string(ROOT_AGGREGATE_VERSION_DEFINITION), "root-aggregate-version-definition", "cm9vdC1hZ2dyZWdhdGUtdmVyc2lvbi1kZWZpbml0aW9u"_base64);
        test(to_string(AGGREGATE), "aggregate", "YWdncmVnYXRl"_base64);
        test(to_string(AGGREGATE_VERSION_DECLARATIONS), "aggregate-version-declarations", "YWdncmVnYXRlLXZlcnNpb24tZGVjbGFyYXRpb25z"_base64);
        test(to_string(AGGREGATE_VERSION_DEFINITION), "aggregate-version-definition", "YWdncmVnYXRlLXZlcnNpb24tZGVmaW5pdGlvbg=="_base64);
    }

    SUBCASE("command")
    {
        using enum state::wizard::command_et;

        test(to_string(CREATE_ROOT_AGGREGATE), "create root aggregate", "Y3JlYXRlIHJvb3QgYWdncmVnYXRl"_base64);
        test(to_string(CREATE_AGGREGATE), "create aggregate", "Y3JlYXRlIGFnZ3JlZ2F0ZQ=="_base64);
        test(to_string(COPY_AGGREGATE), "copy aggregate", "Y29weSBhZ2dyZWdhdGU="_base64);
        test(to_string(CONTINUE_AGGREGATE), "continue aggregate", "Y29udGludWUgYWdncmVnYXRl"_base64);
        test(to_string(DELETE_AGGREGATE), "delete aggregate", "ZGVsZXRlIGFnZ3JlZ2F0ZQ=="_base64);
        test(to_string(CREATE_VERSION), "create version", "Y3JlYXRlIHZlcnNpb24="_base64);
        test(to_string(CREATE_LABEL), "create label", "Y3JlYXRlIGxhYmVs"_base64);
        test(to_string(CREATE_TAG), "create tag", "Y3JlYXRlIHRhZw=="_base64);
        test(to_string(DELETE_TAG), "delete tag", "ZGVsZXRlIHRhZw=="_base64);
        test(to_string(CREATE_ORDERING), "create ordering", "Y3JlYXRlIG9yZGVyaW5n"_base64);
        test(to_string(CREATE_HASH), "create hash", "Y3JlYXRlIGhhc2g="_base64);
        test(to_string(DELETE_HASH), "delete hash", "ZGVsZXRlIGhhc2g="_base64);
        test(to_string(COLLAPSE_VERSION), "collapse version", "Y29sbGFwc2UgdmVyc2lvbg=="_base64);
        test(to_string(BATCH), "batch", "YmF0Y2g="_base64);
    }

    SUBCASE("version status")
    {
        using enum state::version_status_et;

        test(to_string(DELETED), "deleted", "ZGVsZXRlZA=="_base64);
    }
}

}
