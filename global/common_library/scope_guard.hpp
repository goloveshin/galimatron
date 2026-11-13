#pragma once


namespace global
{

// testing: yes (unit)
//
class scope_guard_t
{
public:
    using now_ft = std::function<std::chrono::system_clock::time_point()>;

    // milliseconds, whole seconds, and the minute with the seconds inside it
    //
    struct duration_t
    {
        std::chrono::milliseconds milliseconds{};
        std::chrono::seconds seconds{};
        std::chrono::minutes minutes{};
        std::chrono::seconds seconds_of_minute{};

        // N min L sec, (NN ms)
        //
        std::string text() const
        {
            return std::format(
                "{} min {} sec, ({} ms)",
                minutes.count(),
                seconds_of_minute.count(),
                milliseconds.count()
            );
        }
    };

public:
    scope_guard_t(
        const std::function<void()>& on_construct_func,
        const std::function<void()>& on_destruct_func,
        // the clock. both ends of the measurement call it
        const now_ft& now = std::chrono::system_clock::now
    )
        : scope_guard_t(
            on_construct_func,
            [on_destruct_func](scope_guard_t&)
            {
                if (on_destruct_func)
                {
                    on_destruct_func();
                }
            },
            now
        )
    {}

    // the destruct function receives this guard. the guard's own name is not
    // available inside the constructor arguments (MSVC C7746)
    //
    scope_guard_t(
        const std::function<void()>& on_construct_func,
        const std::function<void(scope_guard_t&)>& on_destruct_func,
        // the clock. both ends of the measurement call it
        const now_ft& now = std::chrono::system_clock::now
    )
        : on_destruct_func_(on_destruct_func)
        , now_(now)
        , started_(now_())
    {
        if (on_construct_func)
        {
            on_construct_func();
        }
    }

    // passes this guard to the destruct function
    //
    ~scope_guard_t()
    {
        if (on_destruct_func_)
        {
            on_destruct_func_(*this);
        }
    }

    scope_guard_t(const scope_guard_t&) = delete;
    scope_guard_t& operator=(const scope_guard_t&) = delete;
    scope_guard_t(scope_guard_t&&) = delete;
    scope_guard_t& operator=(scope_guard_t&&) = delete;

    // time from construction to this call
    //
    duration_t duration() const
    {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now_() - started_);
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
        return {
            .milliseconds = elapsed,
            .seconds = seconds,
            .minutes = minutes,
            .seconds_of_minute = seconds - minutes
        };
    }

private:
    std::function<void(scope_guard_t&)> on_destruct_func_;
    now_ft now_;
    std::chrono::system_clock::time_point started_;
};

}// namespace global
