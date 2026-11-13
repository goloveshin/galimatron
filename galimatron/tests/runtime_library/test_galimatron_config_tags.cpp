// galimatron
#include "galimatron/runtime_library/galimatron_config/detail/galimatron_config_tags.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"

// Base64
#include "base64/base64.hpp"

namespace config = galimatron::translator::galimatron_config;

TEST_SUITE("galimatron/runtime_library/galimatron_config_tags" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("galimatron configuration tags")
{
    auto test = [&](const auto& actual, const auto& expected, const auto& expected_base64)
    {
        CHECK(actual == expected);
        CHECK(base64::encode(actual) == expected_base64);
    };

    SUBCASE("galimatron")
    {
        using namespace config::detail::galimatron_ns;

        test(tag_input_gal_dir, "input-gal-dir", "aW5wdXQtZ2FsLWRpcg=="_base64);
        test(tag_output_gal_dir, "output-gal-dir", "b3V0cHV0LWdhbC1kaXI="_base64);
        test(tag_intermediate_xml_dir, "intermediate-xml-dir", "aW50ZXJtZWRpYXRlLXhtbC1kaXI="_base64);
        test(tag_intermediate_log_dir, "intermediate-log-dir", "aW50ZXJtZWRpYXRlLWxvZy1kaXI="_base64);
        test(tag_intermediate_summary_dir, "intermediate-summary-dir", "aW50ZXJtZWRpYXRlLXN1bW1hcnktZGly"_base64);
        test(tag_committed_report, "committed-report", "Y29tbWl0dGVkLXJlcG9ydA=="_base64);
    }

    SUBCASE("Saxon processor")
    {
        using namespace config::detail::saxon_processor_ns;

        test(tag_working_dir, "working-dir", "d29ya2luZy1kaXI="_base64);
        test(tag_config_file, "config-file", "Y29uZmlnLWZpbGU="_base64);
    }

    SUBCASE("transform input")
    {
        using namespace config::detail::transform::input;

        SUBCASE("principal XSL")
        {
            using namespace principal_xsl_ns;
            test(tag_file, "file", "ZmlsZQ=="_base64);
            test(tag_parameters, "parameters", "cGFyYW1ldGVycw=="_base64);

            SUBCASE("XSL parameters")
            {
                using namespace xsl_parameters_ns;
                test(tag_name, "name", "bmFtZQ=="_base64);
                test(tag_value, "value", "dmFsdWU="_base64);
            }
        }

        SUBCASE("input")
        {
            using namespace input_ns;
            test(tag_principal_xsl, "principal-xsl", "cHJpbmNpcGFsLXhzbA=="_base64);
        }
    }

    SUBCASE("transform output")
    {
        using namespace config::detail::transform::output;

        SUBCASE("principal XML")
        {
            using namespace principal_xml_ns;
            test(tag_file, "file", "ZmlsZQ=="_base64);
        }

        SUBCASE("other files")
        {
            using namespace other_files_ns;
            test(tag_store_dir, "store-dir", "c3RvcmUtZGly"_base64);
            test(tag_store_dir_string_alias, "store-dir-string-alias", "c3RvcmUtZGlyLXN0cmluZy1hbGlhcw=="_base64);
            test(tag_extentions, "extentions", "ZXh0ZW50aW9ucw=="_base64);
            test(tag_replace_escaped_form_to_original_symbols, "replace-escaped-form-to-original-symbols", "cmVwbGFjZS1lc2NhcGVkLWZvcm0tdG8tb3JpZ2luYWwtc3ltYm9scw=="_base64);
            test(tag_delete_if_not_rewrited, "delete-if-not-rewrited", "ZGVsZXRlLWlmLW5vdC1yZXdyaXRlZA=="_base64);
        }

        SUBCASE("output")
        {
            using namespace output_ns;
            test(tag_principal_xml, "principal-xml", "cHJpbmNpcGFsLXhtbA=="_base64);
            test(tag_other_files, "other-files", "b3RoZXItZmlsZXM="_base64);
        }
    }

    SUBCASE("transform")
    {
        using namespace config::detail::transform::transform_ns;

        test(tag_enable, "enable", "ZW5hYmxl"_base64);
        test(tag_name, "name", "bmFtZQ=="_base64);
        test(tag_user_data_for_info, "user-data-for-info", "dXNlci1kYXRhLWZvci1pbmZv"_base64);
        test(tag_input, "input", "aW5wdXQ="_base64);
        test(tag_output, "output", "b3V0cHV0"_base64);
    }

    SUBCASE("transforms")
    {
        using namespace config::detail::transforms;

        SUBCASE("user")
        {
            using namespace user_transforms_ns;
            test(tag_transforms, "transforms", "dHJhbnNmb3Jtcw=="_base64);
        }

        SUBCASE("language")
        {
            using namespace language_transforms_ns;
            test(tag_buildin_transforms, "buildin-transforms", "YnVpbGRpbi10cmFuc2Zvcm1z"_base64);
            test(tag_user_transforms, "user-transforms", "dXNlci10cmFuc2Zvcm1z"_base64);
        }

        SUBCASE("post language")
        {
            using namespace post_language_transforms_ns;
            test(tag_user_transforms, "user-transforms", "dXNlci10cmFuc2Zvcm1z"_base64);
        }

        SUBCASE("transforms")
        {
            using namespace transforms_ns;
            test(tag_language_transforms, "language-transforms", "bGFuZ3VhZ2UtdHJhbnNmb3Jtcw=="_base64);
            test(tag_post_language_transforms, "post-language-transforms", "cG9zdC1sYW5ndWFnZS10cmFuc2Zvcm1z"_base64);
        }
    }

    SUBCASE("configuration")
    {
        using namespace config::config_ns;

        test(tag_galimatron, "galimatron", "Z2FsaW1hdHJvbg=="_base64);
        test(tag_saxon_processor, "saxon-processor", "c2F4b24tcHJvY2Vzc29y"_base64);
        test(tag_transforms, "transforms", "dHJhbnNmb3Jtcw=="_base64);
    }
}

}
