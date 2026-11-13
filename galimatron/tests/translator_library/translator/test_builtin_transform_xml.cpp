// galimatron
#include "galimatron/translator_library/translator/builtin_transform_xml.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

using namespace galimatron::translator::translator;

TEST_SUITE("galimatron/translator_library/translator/builtin_transform_xml" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("file names")
{
    using namespace builtin_transform_xml;

    auto test = [&](
        const auto& actual,
        const auto& expected,
        const auto& expected_base64
    )
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    test(gal2cpp, "lang-parsed.xml", "bGFuZy1wYXJzZWQueG1s"_base64);
    test(gal2cpp_description, "after parsing, before the wizard", "YWZ0ZXIgcGFyc2luZywgYmVmb3JlIHRoZSB3aXphcmQ="_base64);
    test(cpp2xml, "lang-wizard.xml", "bGFuZy13aXphcmQueG1s"_base64);
    test(cpp2xml_description, "after the wizard", "YWZ0ZXIgdGhlIHdpemFyZA=="_base64);
    test(xml2cpp, "lang-xslt.xml", "bGFuZy14c2x0LnhtbA=="_base64);
    test(xml2cpp_description, "after the user xslt, before the seal", "YWZ0ZXIgdGhlIHVzZXIgeHNsdCwgYmVmb3JlIHRoZSBzZWFs"_base64);
    test(cpp2gal, "lang-final.xml", "bGFuZy1maW5hbC54bWw="_base64);
    test(cpp2post_xml, "post-compiled.xml", "cG9zdC1jb21waWxlZC54bWw="_base64);
    test(sealed_description, "after the seal", "YWZ0ZXIgdGhlIHNlYWw="_base64);
}

}
