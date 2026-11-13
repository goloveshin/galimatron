// galimatron
#include "galimatron/tests/forward.hpp"

// Logme
#include <Logme/Logme.h>

namespace
{

// removes Logme channels
//
// testing: no
//
void disable_all_logging()
{
    if (!Logme::Instance)
    {
        return;
    }

    // do not remove the Default channel: use_console() takes it and crashes if it is absent.
    Logme::Instance->IterateChannels([](const std::string&, const Logme::ChannelPtr& channel)
    {
        channel->RemoveBackends();
        channel->SetEnabled(false);
    });
}

// at each test case, and again when doctest reenters it for the next subcase,
// drops every Logme backend and disables the channels.
// this closes the previous case's file log before the next subcase writes
// its default-console line, the read of the next log.cfg, and the reconfig line
// into that intermediate/galimatron.log.
// translate() turns logging back on for the case that is about to run.
// the default channel stays: use_console() crashes if it is gone.
//
// testing: no
//
struct disable_all_logging_listener_t : doctest::IReporter
{
    explicit disable_all_logging_listener_t(const doctest::ContextOptions&) {}

    void report_query(const doctest::QueryData&) override {}
    void test_run_start() override {}
    void test_run_end(const doctest::TestRunStats&) override {}
    void test_case_start(const doctest::TestCaseData&) override
    {
        disable_all_logging();
    }
    void test_case_reenter(const doctest::TestCaseData&) override
    {
        disable_all_logging();
    }
    void test_case_end(const doctest::CurrentTestCaseStats&) override {}
    void test_case_exception(const doctest::TestCaseException&) override {}
    void subcase_start(const doctest::SubcaseSignature&) override {}
    void subcase_end() override {}
    void log_assert(const doctest::AssertData&) override {}
    void log_message(const doctest::MessageData&) override {}
    void test_case_skipped(const doctest::TestCaseData&) override {}
};

REGISTER_LISTENER("disable_all_logging", 1, disable_all_logging_listener_t);

}// namespace
