// galimatron
#include "galimatron/runtime_library/galimatron_config/galimatron_config.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace config = galimatron::translator::galimatron_config;
namespace runtime_config = galimatron::runtime::galimatron_config;

namespace
{

config::transform_t make_transform()
{
    return
    {
        .name = "user-transform",
        .user_data_for_info = "user-data",
        .input =
        {
            .principal_xsl =
            {
                .file =
                {
                    .deferred_content = []{ return galimatron::make_file_content("xsl"); },
                    .path = "transform.xsl"
                },
                .parameters =
                {
                    { .name = "parameter-name", .value = "parameter-value" }
                }
            }
        },
        .output =
        {
            .principal_xml = { .file = "output.xml" },
            .other_files =
            {
                config::detail::transform::output::other_files_t
                {
                    .store_dir = "store",
                    .store_dir_string_alias = "store-alias",
                    .extentions = { ".one", ".two" },
                    .replace_escaped_form_to_original_symbols = true,
                    .delete_if_not_rewrited = false
                }
            }
        }
    };
}

config::config_t make_config()
{
    auto language = make_transform();
    auto post = make_transform();
    post.output.other_files.front().store_dir_string_alias = "post-alias";

    return
    {
        .galimatron =
        {
            .input_gal_dir = "input",
            .output_gal_dir = "output",
            .intermediate_xml_dir = "intermediate-xml",
            .intermediate_log_dir = "intermediate-log",
            .intermediate_summary_dir = "intermediate-summary"
        },
        .saxon_processor =
        {
            .working_dir = "saxon",
            .config_file = "saxon-config.xsl"
        },
        .transforms =
        {
            .language_transforms =
            {
                .buildin_transforms = {},
                .user_transforms = { .transforms = { language } }
            },
            .post_language_transforms = { .user_transforms = { .transforms = { post } } }
        }
    };
}

}// namespace

TEST_SUITE("galimatron/runtime_library/galimatron_config" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("write and read preserve every configuration field")
{
    const auto source = make_config();
    const auto json = runtime_config::write_to_json(source);
    const galimatron::path_t config_file = "/config/galimatron.json";
    const auto actual = runtime_config::read_from_json(
        {
            .content = galimatron::make_file_content(json),
            .path = config_file
        },
        [](const auto& path)
        {
            return galimatron::make_file_content(path.filename().string());
        }
    );

    // Same resolution as read_from_json: absolute() adds the Windows drive.
    const auto config_dir = galimatron::path_t{config_file}.remove_filename();
    const auto expected_path = [&](const galimatron::path_t& relative)
    {
        return std::filesystem::absolute(config_dir / relative).lexically_normal();
    };

    CHECK(actual.galimatron.input_gal_dir == expected_path("input"));
    CHECK(actual.galimatron.output_gal_dir == expected_path("output"));
    CHECK(actual.galimatron.intermediate_xml_dir == expected_path("intermediate-xml"));
    CHECK(actual.galimatron.intermediate_log_dir == expected_path("intermediate-log"));
    CHECK(actual.galimatron.intermediate_summary_dir == expected_path("intermediate-summary"));
    CHECK(actual.galimatron.committed_report == false);
    CHECK(actual.config_dir == std::filesystem::absolute(config_dir).lexically_normal());
    CHECK(actual.saxon_processor.working_dir == expected_path("saxon"));
    CHECK(actual.saxon_processor.config_file == expected_path("saxon/saxon-config.xsl"));

    const auto& actual_transform = actual.transforms.language_transforms.user_transforms.transforms.front();
    CHECK(actual_transform.enable == true);
    CHECK(actual_transform.name == "user-transform");
    CHECK(actual_transform.user_data_for_info == "user-data");
    CHECK(actual_transform.input.principal_xsl.file.path == expected_path("saxon/transform.xsl"));
    CHECK(*actual_transform.input.principal_xsl.file.deferred_content() == "transform.xsl");
    CHECK(actual_transform.input.principal_xsl.parameters.front().name == "parameter-name");
    CHECK(actual_transform.input.principal_xsl.parameters.front().value == "parameter-value");
    CHECK(actual_transform.output.principal_xml.file == galimatron::path_ot{ expected_path("intermediate-xml/output.xml") });
    REQUIRE(actual_transform.output.other_files.size() == 1);
    const auto& other_files = actual_transform.output.other_files.front();
    CHECK(other_files.store_dir == expected_path("store"));
    CHECK(other_files.store_dir_string_alias == "store-alias");
    CHECK(other_files.extentions == std::set<galimatron::path_t>{ ".one", ".two" });
    CHECK(other_files.replace_escaped_form_to_original_symbols == true);
    CHECK(other_files.delete_if_not_rewrited == false);

    CHECK(actual.transforms.post_language_transforms.user_transforms.transforms.size() == 1);
    const auto& actual_post_transform = actual.transforms.post_language_transforms.user_transforms.transforms.front();
    CHECK(actual_post_transform.enable == true);
    CHECK(actual_post_transform.name == "user-transform");
    CHECK(actual_post_transform.user_data_for_info == "user-data");
    CHECK(actual_post_transform.input.principal_xsl.file.path == expected_path("saxon/transform.xsl"));
    CHECK(actual_post_transform.input.principal_xsl.parameters.front().name == "parameter-name");
    CHECK(actual_post_transform.input.principal_xsl.parameters.front().value == "parameter-value");
    CHECK(actual_post_transform.output.principal_xml.file == galimatron::path_ot{ expected_path("intermediate-xml/output.xml") });
    REQUIRE(actual_post_transform.output.other_files.size() == 1);
    const auto& post_other_files = actual_post_transform.output.other_files.front();
    CHECK(post_other_files.store_dir == expected_path("store"));
    CHECK(post_other_files.store_dir_string_alias == "post-alias");
    CHECK(post_other_files.extentions == std::set<galimatron::path_t>{ ".one", ".two" });
    CHECK(post_other_files.replace_escaped_form_to_original_symbols == true);
    CHECK(post_other_files.delete_if_not_rewrited == false);
}

TEST_CASE("delete-if-not-rewrited defaults to true")
{
    auto doc = nlohmann::json::parse(runtime_config::write_to_json(make_config()));
    auto erase_flag = [](auto& node, auto&& self) -> void
    {
        if (node.is_object())
        {
            node.erase("delete-if-not-rewrited");
            for (auto& child : node)
            {
                self(child, self);
            }
        }
        else if (node.is_array())
        {
            for (auto& child : node)
            {
                self(child, self);
            }
        }
    };
    erase_flag(doc, erase_flag);

    const auto actual = runtime_config::read_from_json(
        {
            .content = galimatron::make_file_content(doc.dump()),
            .path = "/config/galimatron.json"
        },
        [](const auto& path)
        {
            return galimatron::make_file_content(path.filename().string());
        }
    );

    const auto& other_files = actual.transforms.language_transforms.user_transforms.transforms.front().output.other_files.front();
    CHECK(other_files.delete_if_not_rewrited == true);
}

TEST_CASE("enable defaults to true")
{
    auto doc = nlohmann::json::parse(runtime_config::write_to_json(make_config()));
    auto erase_flag = [](auto& node, auto&& self) -> void
    {
        if (node.is_object())
        {
            node.erase("enable");
            for (auto& child : node)
            {
                self(child, self);
            }
        }
        else if (node.is_array())
        {
            for (auto& child : node)
            {
                self(child, self);
            }
        }
    };
    erase_flag(doc, erase_flag);

    const auto actual = runtime_config::read_from_json(
        {
            .content = galimatron::make_file_content(doc.dump()),
            .path = "/config/galimatron.json"
        },
        [](const auto& path)
        {
            return galimatron::make_file_content(path.filename().string());
        }
    );

    CHECK(actual.transforms.language_transforms.user_transforms.transforms.front().enable == true);
    CHECK(actual.transforms.post_language_transforms.user_transforms.transforms.front().enable == true);
}

TEST_CASE("committed-report defaults to false")
{
    auto doc = nlohmann::json::parse(runtime_config::write_to_json(make_config()));
    doc["galimatron"].erase("committed-report");

    const auto actual = runtime_config::read_from_json(
        {
            .content = galimatron::make_file_content(doc.dump()),
            .path = "/config/galimatron.json"
        },
        [](const auto& path)
        {
            return galimatron::make_file_content(path.filename().string());
        }
    );

    CHECK(actual.galimatron.committed_report == false);
}

TEST_CASE("committed-report true is preserved")
{
    auto source = make_config();
    source.galimatron.committed_report = true;

    const auto actual = runtime_config::read_from_json(
        {
            .content = galimatron::make_file_content(runtime_config::write_to_json(source)),
            .path = "/config/galimatron.json"
        },
        [](const auto& path)
        {
            return galimatron::make_file_content(path.filename().string());
        }
    );

    CHECK(actual.galimatron.committed_report == true);
}

TEST_CASE("committed-report spells a path relative to the gal.cfg directory")
{
    config::config_t config;
    config.galimatron.committed_report = true;
    config.config_dir = std::filesystem::absolute("generated");
    const auto file = config.config_dir / "intermediate" / "lang-parsed.xml";
    CHECK(config::report_path(config, file) == "intermediate/lang-parsed.xml");
    const auto directory = config.config_dir / "incoming_gal" / "";
    CHECK(config::report_path(config, directory) == "incoming_gal");

    config.galimatron.committed_report = false;
    CHECK(config::report_path(config, file) == file.string());
}

TEST_CASE("copies of the report spell the same path")
{
    config::config_t config;
    config.galimatron.committed_report = true;
    config.config_dir = std::filesystem::absolute("generated");
    const auto file = config.config_dir / "intermediate" / "lang-parsed.xml";
    CHECK(config::report_path(file, true, config.config_dir) == "intermediate/lang-parsed.xml");
    CHECK(config::report_path(file, false, config.config_dir) == file.string());
}

TEST_CASE("the run slot spells a path and restores the previous one")
{
    config::config_t config;
    config.config_dir = std::filesystem::absolute("generated");
    const auto file = config.config_dir / "intermediate" / "lang-parsed.xml";
    const auto before = config::report_path(file);
    {
        const auto guard = galimatron::path_report_guard_t({
            .committed_report = true,
            .config_dir = config.config_dir
        });
        CHECK(config::report_path(file) == "intermediate/lang-parsed.xml");
    }
    CHECK(config::report_path(file) == before);
}

TEST_CASE("enable false is preserved")
{
    auto source = make_config();
    source.transforms.language_transforms.user_transforms.transforms.front().enable = false;
    source.transforms.post_language_transforms.user_transforms.transforms.front().enable = false;

    const auto actual = runtime_config::read_from_json(
        {
            .content = galimatron::make_file_content(runtime_config::write_to_json(source)),
            .path = "/config/galimatron.json"
        },
        [](const auto& path)
        {
            return galimatron::make_file_content(path.filename().string());
        }
    );

    CHECK(actual.transforms.language_transforms.user_transforms.transforms.front().enable == false);
    CHECK(actual.transforms.post_language_transforms.user_transforms.transforms.front().enable == false);
}

}
