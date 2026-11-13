// galimatron
#include "galimatron/runtime_library/translate_summary_xml/translate_summary_xml.hpp"
#include "galimatron/tests/forward.hpp"

// Boost
#include <boost/asio/io_context.hpp>
#include <boost/process/process.hpp>
#if defined(_WIN32)
#include <boost/process/v2/windows/creation_flags.hpp>
#endif


namespace galimatron::runtime::crash
{

namespace
{

std::string read_file(const path_t& path)
{
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    );
}

}// namespace

TEST_SUITE("galimatron/runtime_library/crash" * galimatron::test_axes::mark(galimatron::test_axes::process))
{

// the child process crashes, and the handler writes the log and the summary
//
TEST_CASE("access violation writes the crash log and the summary")
{
    const auto directory = fs::temp_directory_path() / "galimatron_crash_test";
    fs::remove_all(directory);
    fs::create_directories(directory);

    const auto log_path = directory / "galimatron.log";
    const auto crash_log_path = directory / "galimatron.crash.log";
    const auto summary_path = directory / translate_summary_xml::file_name;
    const auto log_arg = log_path.string();
    const auto summary_arg = summary_path.string();

    boost::asio::io_context io_context;
#if defined(_WIN32)
    // the child's access violation must not fail the console that runs the suite
    const auto child_flags = boost::process::v2::windows::process_creation_flags<
        CREATE_BREAKAWAY_FROM_JOB | CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS
    >{};
    boost::process::v2::process child(
        io_context,
        path_to_galimatron_crash_child.string(),
        { log_arg, summary_arg },
        child_flags
    );
#else
    boost::process::v2::process child(
        io_context,
        path_to_galimatron_crash_child.string(),
        { log_arg, summary_arg }
    );
#endif
    child.wait();

    CHECK(child.exit_code() != 0);

    const auto crash_log = read_file(crash_log_path);
    const auto summary = read_file(summary_path);

    CHECK(crash_log.find("crash\n") != std::string::npos);
    CHECK(crash_log.find("stacktrace:\n") != std::string::npos);
    CHECK(summary.find("status=\"crash\"") != std::string::npos);
    CHECK(summary.find("stacktrace:") != std::string::npos);
#if defined(_WIN32)
    CHECK(crash_log.find("exception:") != std::string::npos);
#else
    CHECK(crash_log.find("signal:") != std::string::npos);
#endif
}

}

}// namespace galimatron::runtime::crash
