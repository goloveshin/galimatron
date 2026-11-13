// galimatron
#include "galimatron/translator_library/xml_tags.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

using namespace galimatron::translator;

TEST_SUITE("galimatron/translator_library/xml_tags" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("xml tags")
{
    using namespace xml_tags;

    auto test = [&](
        const auto& actual,
        const auto& expected,
        const auto& expected_base64
    )
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    SUBCASE("common")
    {
        test(tag_c::name, "name", "bmFtZQ=="_base64);
        test(tag_c::index, "index", "aW5kZXg="_base64);
        test(tag_c::owner, "owner", "b3duZXI="_base64);
        test(tag_c::status, "status", "c3RhdHVz"_base64);
        test(tag_c::values, "values", "dmFsdWVz"_base64);
        test(tag_c::value, "value", "dmFsdWU="_base64);
        test(tag_c::string, "string", "c3RyaW5n"_base64);
        test(tag_c::name, "name", "bmFtZQ=="_base64);
        test(tag_c::hash, "hash", "aGFzaA=="_base64);
        test(tag_c::comment, "comment", "Y29tbWVudA=="_base64);
        test(tag_c::mode, "mode", "bW9kZQ=="_base64);
        test(tag_c::layout_mark, "layout-mark", "bGF5b3V0LW1hcms="_base64);
        test(tag_c::delimiter, "delimiter", "ZGVsaW1pdGVy"_base64);
        test(tag_c::raw, "raw", "cmF3"_base64);
        test(tag_c::quote, "quote", "cXVvdGU="_base64);
        test(tag_c::dashes, "dashes", "ZGFzaGVz"_base64);
        test(tag_c::bool_true, "true", "dHJ1ZQ=="_base64);
        test(tag_c::bool_false, "false", "ZmFsc2U="_base64);
    }

    SUBCASE("file")
    {
        test(file::chunk, "chunk", "Y2h1bms="_base64);
        test(file::location, "location", "bG9jYXRpb24="_base64);
        test(file::filesystem, "filesystem", "ZmlsZXN5c3RlbQ=="_base64);
        test(file::path, "path", "cGF0aA=="_base64);

        test(file::root, "root", "cm9vdA=="_base64);
        test(file::relative, "relative", "cmVsYXRpdmU="_base64);
        test(file::relative_and_filename, "relative-and-filename", "cmVsYXRpdmUtYW5kLWZpbGVuYW1l"_base64);
        test(file::stem, "stem", "c3RlbQ=="_base64);
        test(file::extension, "extension", "ZXh0ZW5zaW9u"_base64);
    }

    SUBCASE("xml document")
    {
        test(xml_document::version, "version", "dmVyc2lvbg=="_base64);
        test(xml_document::encoding, "encoding", "ZW5jb2Rpbmc="_base64);
    }
}

}
