// galimatron
#include "galimatron/runtime_library/crash/crash.hpp"
#include "galimatron/runtime_library/translate/translate.hpp"
#include "galimatron/tests/forward.hpp"


namespace galimatron::runtime
{

TEST_SUITE("galimatron/runtime_library/translate" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

// a relative home-directory.path is the directory of log.cfg
// the intermediate folder, galimatron.log, and galimatron.crash.log are created there
//
TEST_CASE("relative home creates the intermediate logs")
{
    const auto config_path = path_to_galimatron_test_data / "translator_library/common/default/log.cfg";
    const auto intermediate = config_path.parent_path() / test_folders::intermediate;
    fs::remove_all(intermediate);

    const auto previous_home = Logme::Instance->GetHomeDirectory();
    auto restore = scope_guard_t(
        []{},
        [&]
        {
            Logme::Instance->CloseCrashLog();
            Logme::Instance->IterateChannels([](const std::string&, const Logme::ChannelPtr& channel)
            {
                channel->RemoveBackends();
                channel->SetEnabled(false);
            });
            Logme::Instance->SetHomeDirectory(previous_home);
            std::error_code error;
            fs::remove_all(intermediate, error);
        }
    );

    CHECK_FALSE(fs::exists(intermediate));

    apply_log_config(translate_t::get_log_config_from_file(config_path));
    const auto crash_log = crash::arm_log();

    CHECK(fs::is_directory(intermediate));
    CHECK(fs::is_regular_file(intermediate / "galimatron.log"));
    REQUIRE(crash_log);
    CHECK(fs::is_regular_file(*crash_log));
    CHECK(fs::equivalent(*crash_log, intermediate / "galimatron.crash.log"));
}

}

}// namespace galimatron::runtime
