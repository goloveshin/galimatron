// global
#include "global/tests/forward.hpp"

// global
#include "global/common_library/scope_guard.hpp"


namespace global
{

TEST_SUITE("global/common_library/scope_guard")
{

TEST_CASE("scope_guard")
{
    int on_construct = 0;
    int on_destruct = 0;

    {
        auto guard = scope_guard_t(
            [&]{ on_construct++; },
            [&]{ on_destruct++; }
        );

        CHECK(on_construct == 1);
        CHECK(on_destruct == 0);
    }

    CHECK(on_construct == 1);
    CHECK(on_destruct == 1);
}

// an injected clock reports 2 min 5 sec
//
TEST_CASE("duration")
{
    using clock = std::chrono::system_clock;
    auto current = clock::time_point{};
    auto now = [&]{ return current; };

    std::string shown;
    {
        auto guard = scope_guard_t(
            []{},
            [&](scope_guard_t& guard)
            {
                shown = guard.duration().text();
            },
            now
        );

        current += std::chrono::milliseconds{ 125000 };
        const auto duration = guard.duration();
        CHECK(duration.milliseconds.count() == 125000);
        CHECK(duration.seconds.count() == 125);
        CHECK(duration.minutes.count() == 2);
        CHECK(duration.seconds_of_minute.count() == 5);
    }

    CHECK(shown == "2 min 5 sec, (125000 ms)");
}

}

}// namespace global
