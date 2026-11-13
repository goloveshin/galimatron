// galimatron
#include "galimatron/runtime_library/crash/crash.hpp"

// global
#include "global/common_library/logging_utils.hpp"

// Logme
#include <Logme/Logme.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
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


// child process: installs the handler, writes the crash log and the summary, then crashes
//
int main(int argc, char* argv[])
{
    if (argc != 3)
        return 2;

#if defined(_WIN32)
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

    galimatron::runtime::crash::install();
    global::logging_utils::use_named_channel("file", argv[1]);

    if (!galimatron::runtime::crash::arm_log())
        return 3;

    // the crash text stays in the log file. stderr would look like the build itself crashed
    Logme::Instance->SetCrashOutputMask(Logme::CRASH_OUTPUT_FILE);

    galimatron::runtime::crash::arm_summary(argv[2]);

#if defined(_WIN32)
    RaiseException(0xC0000005u, 0, 0, nullptr);
#else
    std::raise(SIGSEGV);
#endif
    return 0;
}
