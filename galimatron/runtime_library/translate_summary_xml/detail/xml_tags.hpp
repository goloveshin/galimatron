#pragma once


// testing: yes (unit)
//
namespace galimatron::runtime::transformation_summary_xml::xml_tags
{

static constexpr std::string_view transformations = "transformations";
static constexpr std::string_view transformation = "transformation";
static constexpr std::string_view produced_xml = "produced-xml";
static constexpr std::string_view produced_xml_path = "path";
static constexpr std::string_view produced_files = "produced-files";
static constexpr std::string_view produced_file = "file";
static constexpr std::string_view produced_path = "path";
static constexpr std::string_view index = "index";
static constexpr std::string_view name = "name";
static constexpr std::string_view from = "from";
static constexpr std::string_view build_in = "build-in";
static constexpr std::string_view user = "user";
static constexpr std::string_view count = "count";
static constexpr std::string_view user_data = "user-data";
static constexpr std::string_view status = "status";
static constexpr std::string_view state_bytes = "state-bytes";
static constexpr std::string_view success = "success";
static constexpr std::string_view fail = "fail";
static constexpr std::string_view crash = "crash";
static constexpr std::string_view error = "error";

}// namespace galimatron::runtime::transformation_summary_xml
