#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


// testing: yes (unit)
//
namespace galimatron::translator::galimatron_config
{

namespace detail
{

namespace galimatron_ns
{

static constexpr std::string_view tag_input_gal_dir = "input-gal-dir";
static constexpr std::string_view tag_output_gal_dir = "output-gal-dir";
static constexpr std::string_view tag_intermediate_xml_dir = "intermediate-xml-dir";
static constexpr std::string_view tag_intermediate_log_dir = "intermediate-log-dir";
static constexpr std::string_view tag_intermediate_summary_dir = "intermediate-summary-dir";
static constexpr std::string_view tag_committed_report = "committed-report";

}// namespace galimatron_ns

namespace saxon_processor_ns
{

static constexpr std::string_view tag_working_dir = "working-dir";
static constexpr std::string_view tag_config_file = "config-file";

}// namespace saxon_processor_ns

namespace transform
{

namespace input
{

namespace principal_xsl_ns
{

namespace xsl_parameters_ns
{

static constexpr std::string_view tag_name = "name";
static constexpr std::string_view tag_value = "value";

}// namespace xsl_parameters_ns

static constexpr std::string_view tag_file = "file";
static constexpr std::string_view tag_parameters = "parameters";

}// namespace principal_xsl_ns

namespace input_ns
{

static constexpr std::string_view tag_principal_xsl = "principal-xsl";

}// namespace input_ns

}// namespace input

namespace output
{

namespace principal_xml_ns
{

static constexpr std::string_view tag_file = "file";

}// namespace principal_xml_ns

namespace other_files_ns
{

static constexpr std::string_view tag_store_dir = "store-dir";
static constexpr std::string_view tag_store_dir_string_alias = "store-dir-string-alias";
static constexpr std::string_view tag_extentions = "extentions";
static constexpr std::string_view tag_replace_escaped_form_to_original_symbols = "replace-escaped-form-to-original-symbols";
static constexpr std::string_view tag_delete_if_not_rewrited = "delete-if-not-rewrited";

}// namespace other_files_ns

namespace output_ns
{

static constexpr std::string_view tag_principal_xml = "principal-xml";
static constexpr std::string_view tag_other_files = "other-files";

}// namespace output_ns

}// namespace output

namespace transform_ns
{

static constexpr std::string_view tag_enable = "enable";
static constexpr std::string_view tag_name = "name";
static constexpr std::string_view tag_user_data_for_info = "user-data-for-info";
static constexpr std::string_view tag_input = "input";
static constexpr std::string_view tag_output = "output";

}// namespace transform

}// namespace transform

namespace transforms
{

namespace user_transforms_ns
{

static constexpr std::string_view tag_transforms = "transforms";

}// namespace user_transforms_ns

namespace language_transforms_ns
{

static constexpr std::string_view tag_buildin_transforms = "buildin-transforms";
static constexpr std::string_view tag_user_transforms = "user-transforms";

}// namespace language_transforms_ns

namespace post_language_transforms_ns
{

static constexpr std::string_view tag_user_transforms = "user-transforms";

}// namespace post_language_transforms_ns

namespace transforms_ns
{

static constexpr std::string_view tag_language_transforms = "language-transforms";
static constexpr std::string_view tag_post_language_transforms = "post-language-transforms";

}// namespace transforms_ns

}// namespace transforms

}// namespace detail

namespace config_ns
{

static constexpr std::string_view tag_galimatron = "galimatron";
static constexpr std::string_view tag_saxon_processor = "saxon-processor";
static constexpr std::string_view tag_transforms = "transforms";

}// namespace config_ns

}// namespace galimatron::translator::galimatron_config
