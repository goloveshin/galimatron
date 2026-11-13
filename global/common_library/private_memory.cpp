// global
#include "global/common_library/private_memory.hpp"
#include "global/common_library/type_declaration.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#ifdef ERROR
#undef ERROR
#endif
#ifdef DELETE
#undef DELETE
#endif
#ifdef EOF
#undef EOF
#endif
#endif


namespace global
{

#if defined(__linux__)
namespace
{

std::optional<std::size_t> rss_anon_bytes()
{
    std::ifstream input("/proc/self/status");
    std::string line;
    while (std::getline(input, line))
    {
        if (!boost::starts_with(line, "RssAnon:"))
        {
            continue;
        }

        // "RssAnon:    12345 kB"
        strings_t parts;
        boost::split(parts, line, boost::is_any_of(" \t"), boost::token_compress_on);
        if (parts.size() < 2 || parts[1].empty())
        {
            return {};
        }

        const auto& number = parts[1];
        const auto digits = ranges::all_of(
            number,
            [](const char symbol)
            {
                return std::isdigit(static_cast<unsigned char>(symbol));
            }
        );
        if (!digits)
        {
            return {};
        }

        return static_cast<std::size_t>(std::stoull(number) * 1024);
    }

    return {};
}

}// namespace
#endif


std::optional<std::size_t> private_memory_bytes()
{
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX counters = {};
    counters.cb = sizeof(counters);

    if (!GetProcessMemoryInfo(
        GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
        sizeof(counters)
    ))
    {
        return {};
    }

    return counters.PrivateUsage;
#elif defined(__linux__)
    return rss_anon_bytes();
#else
    return {};
#endif
}

}// namespace global
