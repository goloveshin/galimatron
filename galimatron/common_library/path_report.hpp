#pragma once

// galimatron
#include "galimatron/common_library/type_declaration.hpp"


namespace galimatron
{

// how this run spells a path. workers read it; the catch does not
//
struct path_report_t
{
    bool committed_report = false;
    path_t config_dir;
};

// one run at a time. the compiler and the validator log from taskflow workers
//
inline path_report_t& current_path_report()
{
    static path_report_t report;
    return report;
}

// saves the current report and installs next. restores on the way out
//
class path_report_guard_t
{
public:
    explicit path_report_guard_t(const path_report_t& next)
        : previous_(current_path_report())
    {
        current_path_report() = next;
    }

    ~path_report_guard_t()
    {
        current_path_report() = previous_;
    }

    path_report_guard_t(const path_report_guard_t&) = delete;
    path_report_guard_t& operator=(const path_report_guard_t&) = delete;

private:
    path_report_t previous_;
};

}// namespace galimatron
