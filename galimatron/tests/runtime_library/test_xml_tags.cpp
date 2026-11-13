// galimatron
#include "galimatron/runtime_library/translate_summary_xml/detail/xml_tags.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

using namespace galimatron::runtime;

TEST_SUITE("galimatron/runtime_library/translate_summary_xml/detail/xml_tags" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("transformation summary xml tags")
{
    using namespace transformation_summary_xml::xml_tags;

    auto test = [&](
        const auto& actual,
        const auto& expected,
        const auto& expected_base64
    )
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    test(transformations, "transformations", "dHJhbnNmb3JtYXRpb25z"_base64);
    test(transformation, "transformation", "dHJhbnNmb3JtYXRpb24="_base64);
    test(produced_xml, "produced-xml", "cHJvZHVjZWQteG1s"_base64);
    test(produced_xml_path, "path", "cGF0aA=="_base64);
    test(produced_files, "produced-files", "cHJvZHVjZWQtZmlsZXM="_base64);
    test(produced_file, "file", "ZmlsZQ=="_base64);
    test(transformation_summary_xml::xml_tags::index, "index", "aW5kZXg="_base64);
    test(name, "name", "bmFtZQ=="_base64);
    test(from, "from", "ZnJvbQ=="_base64);
    test(build_in, "build-in", "YnVpbGQtaW4="_base64);
    test(user, "user", "dXNlcg=="_base64);
    test(count, "count", "Y291bnQ="_base64);
    test(user_data, "user-data", "dXNlci1kYXRh"_base64);
    test(status, "status", "c3RhdHVz"_base64);
    test(state_bytes, "state-bytes", "c3RhdGUtYnl0ZXM="_base64);
    test(success, "success", "c3VjY2Vzcw=="_base64);
    test(fail, "fail", "ZmFpbA=="_base64);
    test(crash, "crash", "Y3Jhc2g="_base64);
    test(error, "error", "ZXJyb3I="_base64);
}

}
