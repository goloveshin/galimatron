// galimatron
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

using namespace galimatron::translator::state;

TEST_SUITE("galimatron/translator_library/state/gal_and_xml_tags" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("gal and xml tags")
{
    using namespace gal_and_xml_tags;

    auto test = [&](
        const auto& actual,
        const auto& expected,
        const auto& expected_base64
    )
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    SUBCASE("document")
    {
        test(document::galimatron, "galimatron", "Z2FsaW1hdHJvbg=="_base64);
        test(document::description, "description", "ZGVzY3JpcHRpb24="_base64);
        test(document::language, "language", "bGFuZ3VhZ2U="_base64);
        test(document::compiled, "compiled", "Y29tcGlsZWQ="_base64);
        test(document::history, "history", "aGlzdG9yeQ=="_base64);
        test(document::string_delimiters, "string-delimiters", "c3RyaW5nLWRlbGltaXRlcnM="_base64);
        test(document::string_delimiter, "string-delimiter", "c3RyaW5nLWRlbGltaXRlcg=="_base64);
    }

    SUBCASE("root aggregate")
    {
        test(root_aggregate::root_aggregate, "root-aggregate", "cm9vdC1hZ2dyZWdhdGU="_base64);
        test(root_aggregate::root_aggregate_version_declarations, "root-aggregate-version-declarations", "cm9vdC1hZ2dyZWdhdGUtdmVyc2lvbi1kZWNsYXJhdGlvbnM="_base64);
        test(root_aggregate::root_aggregate_version_declaration, "root-aggregate-version-declaration", "cm9vdC1hZ2dyZWdhdGUtdmVyc2lvbi1kZWNsYXJhdGlvbg=="_base64);
        test(root_aggregate::root_aggregate_version_definitions, "root-aggregate-version-definitions", "cm9vdC1hZ2dyZWdhdGUtdmVyc2lvbi1kZWZpbml0aW9ucw=="_base64);
        test(root_aggregate::root_aggregate_version_definition, "root-aggregate-version-definition", "cm9vdC1hZ2dyZWdhdGUtdmVyc2lvbi1kZWZpbml0aW9u"_base64);

        test(root_aggregate_gal::root_aggregate, "root aggregate", "cm9vdCBhZ2dyZWdhdGU="_base64);
        test(root_aggregate_gal::versions, "versions", "dmVyc2lvbnM="_base64);
        test(root_aggregate_gal::version, "version", "dmVyc2lvbg=="_base64);
        test(root_aggregate_gal::before, "before", "YmVmb3Jl"_base64);
    }

    SUBCASE("aggregate")
    {
        test(aggregate::aggregate, "aggregate", "YWdncmVnYXRl"_base64);
        test(aggregate::aggregate_version_declarations, "aggregate-version-declarations", "YWdncmVnYXRlLXZlcnNpb24tZGVjbGFyYXRpb25z"_base64);
        test(aggregate::aggregate_version_declaration, "aggregate-version-declaration", "YWdncmVnYXRlLXZlcnNpb24tZGVjbGFyYXRpb24="_base64);
        test(aggregate::aggregate_version_definitions, "aggregate-version-definitions", "YWdncmVnYXRlLXZlcnNpb24tZGVmaW5pdGlvbnM="_base64);
        test(aggregate::aggregate_version_definition, "aggregate-version-definition", "YWdncmVnYXRlLXZlcnNpb24tZGVmaW5pdGlvbg=="_base64);
        test(aggregate::orderings, "orderings", "b3JkZXJpbmdz"_base64);
        test(aggregate::ordering, "ordering", "b3JkZXJpbmc="_base64);

        test(aggregate_gal::aggregate, "aggregate", "YWdncmVnYXRl"_base64);
        test(aggregate_gal::versions, "versions", "dmVyc2lvbnM="_base64);
        test(aggregate_gal::version, "version", "dmVyc2lvbg=="_base64);
        test(aggregate_gal::before, "before", "YmVmb3Jl"_base64);
    }

    SUBCASE("tag")
    {
        test(tag::tag, "tag", "dGFn"_base64);
        test(tag::label_refs, "label-refs", "bGFiZWwtcmVmcw=="_base64);
        test(tag::label_ref, "label-ref", "bGFiZWwtcmVm"_base64);
        test(tag::label_static, "static", "c3RhdGlj"_base64);
        test(tag::label_dynamic, "dynamic", "ZHluYW1pYw=="_base64);
        test(tag::values, "values", "dmFsdWVz"_base64);
        test(tag::value, "value", "dmFsdWU="_base64);
        test(tag::record, "record", "cmVjb3Jk"_base64);
        test(tag::array, "array", "YXJyYXk="_base64);
        test(tag::key, "key", "a2V5"_base64);

        test(tag_gal::once, "once", "b25jZQ=="_base64);
        test(tag_gal::local, "local", "bG9jYWw="_base64);
        test(tag_gal::own, "own", "b3du"_base64);
    }

    SUBCASE("label")
    {
        test(label::label, "label", "bGFiZWw="_base64);
        test(label::reference, "reference", "cmVmZXJlbmNl"_base64);
        test(label::agg_is_root, "agg-is-root", "YWdnLWlzLXJvb3Q="_base64);
        test(label::agg_name, "agg-name", "YWdnLW5hbWU="_base64);
        test(label::agg_index, "agg-index", "YWdnLWluZGV4"_base64);
        test(label::dynamic_highlight, "dynamic", "ZHluYW1pYw=="_base64);
        test(label::static_highlight, "static", "c3RhdGlj"_base64);
        test(label::important, "important", "aW1wb3J0YW50"_base64);
        test(label::values, "values", "dmFsdWVz"_base64);
        test(label::value, "value", "dmFsdWU="_base64);
        test(label::tag_name_from, "tag-name-from", "dGFnLW5hbWUtZnJvbQ=="_base64);

        test(label_gal::label, "label", "bGFiZWw="_base64);
        test(label_gal::static_highlight, "static", "c3RhdGlj"_base64);
        test(label_gal::tag_name_from, "tag-name-from", "dGFnLW5hbWUtZnJvbQ=="_base64);
    }

    SUBCASE("wizard mark")
    {
        test(wizard_mark::dynamic, "dynamic", "ZHluYW1pYw=="_base64);
        test(wizard_mark::wizard, "wizard", "d2l6YXJk"_base64);
        test(wizard_mark::new_root_aggregate, "new-root-aggregate", "bmV3LXJvb3QtYWdncmVnYXRl"_base64);
        test(wizard_mark::new_aggregate, "new-aggregate", "bmV3LWFnZ3JlZ2F0ZQ=="_base64);
        test(wizard_mark::new_version, "new-version", "bmV3LXZlcnNpb24="_base64);
    }

    SUBCASE("operation")
    {
        test(operation::delete_tag, "delete", "ZGVsZXRl"_base64);
        test(operation_gal::delete_tag, "delete", "ZGVsZXRl"_base64);
    }

    SUBCASE("ordering")
    {
        test(ordering::ordering, "ordering", "b3JkZXJpbmc="_base64);
        test(ordering::ordering_parts, "ordering-parts", "b3JkZXJpbmctcGFydHM="_base64);
        test(ordering::ordering_part, "ordering-part", "b3JkZXJpbmctcGFydA=="_base64);
        test(ordering::aggregate_ref, "aggregate-ref", "YWdncmVnYXRlLXJlZg=="_base64);
        test(ordering::name, "name", "bmFtZQ=="_base64);

        test(ordering_gal::ordering, "ordering", "b3JkZXJpbmc="_base64);
    }

    SUBCASE("GaL punctuation")
    {
        test(GaL::COL, ":", "Og=="_base64);
        test(GaL::SEMI, ";", "Ow=="_base64);
        test(GaL::LC, "{", "ew=="_base64);
        test(GaL::RC, "}", "fQ=="_base64);
        test(GaL::LP, "(", "KA=="_base64);
        test(GaL::RP, ")", "KQ=="_base64);
        test(GaL::LS, "[", "Ww=="_base64);
        test(GaL::RS, "]", "XQ=="_base64);
        test(GaL::EQ, "=", "PQ=="_base64);
        test(GaL::DOT, ".", "Lg=="_base64);
        test(GaL::COMMA, ",", "LA=="_base64);
        test(GaL::COMMA_WITH_SPACE, ", ", "LCA="_base64);
        test(GaL::NEWLINE, "\n", "Cg=="_base64);
        test(GaL::CR, "\r", "DQ=="_base64);
        test(GaL::CRLF, "\r\n", "DQo="_base64);
        test(GaL::SPACE, " ", "IA=="_base64);
        test(GaL::CPP_COMMENT, "//", "Ly8="_base64);
        test(GaL::C_COMMENT, "/*", "Lyo="_base64);
        test(GaL::CPP_COMMENT_WITH_SPACE, "// ", "Ly8g"_base64);
    }
}

}
