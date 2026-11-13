#pragma once

// directory names of a Galimatron test case
// production gal.cfg chooses its own directories, so these names are the test layout only
// expected-* is the oracle and exists only in tests
// test gal.cfg repeats incoming_gal, actual_gal, actual_xml, actual_log, intermediate, and actual_summary
//
namespace galimatron::test_folders
{

inline constexpr std::string_view incoming_gal = "incoming_gal";
inline constexpr std::string_view actual_gal = "actual_gal";
inline constexpr std::string_view actual_xml = "actual_xml";
inline constexpr std::string_view actual_log = "actual_log";
inline constexpr std::string_view intermediate = "intermediate";
inline constexpr std::string_view actual_summary = "actual_summary";
inline constexpr std::string_view expected_gal = "expected-gal";
inline constexpr std::string_view expected_xml = "expected-xml";
inline constexpr std::string_view expected_log = "expected-log";

}// namespace galimatron::test_folders
