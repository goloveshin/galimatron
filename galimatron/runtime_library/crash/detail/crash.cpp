// galimatron
#include "galimatron/runtime_library/crash/crash.hpp"
#include "galimatron/runtime_library/translate_summary_xml/detail/xml_tags.hpp"

// Logme
#include <Logme/Backend/FileBackend.h>
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
#else
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif


namespace galimatron::runtime::crash
{

namespace
{
namespace tag = transformation_summary_xml::xml_tags;

// a second entry from stacktrace collection must not collect the stacktrace again
std::atomic_flag reporting = ATOMIC_FLAG_INIT;

// the path is remembered before the crash. The path allocator is no longer called in the handler.
path_t summary_path;
std::atomic<bool> summary_ready{ false };

// the process exits without C++ destructors:
// the destructor of a normal summary must not overwrite the file written from here
//
void stop_process(unsigned long code)
{
#if defined(_WIN32)
    TerminateProcess(GetCurrentProcess(), static_cast<UINT>(code));
#else
    _exit(128 + static_cast<int>(code));
#endif
}

// CDATA cannot contain the sequence ]]>
//
std::string summary_text(const std::string& body)
{
    std::string escaped;
    escaped.reserve(body.size());
    for (std::size_t i = 0; i < body.size();)
    {
        if (body.compare(i, 3, "]]>") == 0)
        {
            escaped += "]]]]><![CDATA[>";
            i += 3;
        }
        else
        {
            escaped += body[i];
            ++i;
        }
    }

    std::string xml;
    xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<";
    xml += tag::transformations;
    xml += ' ';
    xml += tag::status;
    xml += "=\"";
    xml += tag::crash;
    xml += "\"><";
    xml += tag::error;
    xml += "><![CDATA[\n";
    xml += escaped;
    xml += "\n]]></";
    xml += tag::error;
    xml += "></";
    xml += tag::transformations;
    xml += ">\n";
    return xml;
}

// direct OS write: pugixml and filesystem are not called from the handler
//
void write_summary(const std::string& xml)
{
    // the intermediate files directory has not been created yet
    if (!summary_ready.load(std::memory_order_acquire))
        return;

#if defined(_WIN32)
    HANDLE handle = CreateFileW(
        summary_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (handle == INVALID_HANDLE_VALUE)
        return;

    const char* data = xml.data();
    size_t size = xml.size();
    while (size > 0)
    {
        DWORD chunk = size > static_cast<size_t>(0x7ffff000) ? 0x7ffff000 : static_cast<DWORD>(size);
        DWORD written = 0;
        if (!WriteFile(handle, data, chunk, &written, nullptr) || written == 0)
            break;
        data += written;
        size -= written;
    }
    CloseHandle(handle);
#else
    const int fd = ::open(summary_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
        return;

    const char* data = xml.data();
    size_t size = xml.size();
    while (size > 0)
    {
        const ssize_t written = ::write(fd, data, size);
        if (written <= 0)
            break;
        data += written;
        size -= static_cast<size_t>(written);
    }
    ::close(fd);
#endif
}

// code and address, then std::stacktrace, into the Logme crash log and the summary
//
void report(const char* kind, unsigned long code, const void* address)
{
    // stacktrace itself can crash. A repeated entry only terminates the process.
    if (reporting.test_and_set(std::memory_order_acq_rel))
    {
        stop_process(code);
        return;
    }

    char header[160];
    const int written = std::snprintf(
        header,
        sizeof header,
        "crash\n%s: %#lx\naddress: %p\nstacktrace:\n",
        kind,
        code,
        address
    );

    std::string body;
    if (written > 0)
    {
        const auto length = static_cast<std::size_t>(written) < sizeof header
            ? static_cast<std::size_t>(written)
            : sizeof header - 1;
        body.assign(header, length);
    }

    // std::stacktrace allocates memory. On a heap failure the header remains without frames.
    try
    {
        body += std::to_string(std::stacktrace::current());
    }
    catch (...)
    {
        body += "unavailable\n";
    }
    if (body.empty() || body.back() != '\n')
        body += '\n';

    // CrashWrite does not take channel locks. A normal fLogme cannot be used from here.
    if (Logme::Instance)
        Logme::Instance->CrashWrite(body.data(), body.size());

    write_summary(summary_text(body));
    stop_process(code);
}

#if defined(_WIN32)
// unhandled SEH, including access violation
//
LONG WINAPI on_exception(EXCEPTION_POINTERS* info)
{
    unsigned long code = 0;
    const void* address = nullptr;
    if (info && info->ExceptionRecord)
    {
        code = info->ExceptionRecord->ExceptionCode;
        address = info->ExceptionRecord->ExceptionAddress;
    }
    report("exception", code, address);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
// SIGSEGV, SIGABRT, SIGBUS, SIGFPE, SIGILL
//
void on_signal(int sig, siginfo_t* info, void*)
{
    const void* address = info ? info->si_addr : nullptr;
    report("signal", static_cast<unsigned long>(sig), address);
}
#endif

// the path that FileBackend has already opened, not the path from the log.cfg text
//
path_ot file_backend_path()
{
    path_ot found;
    if (!Logme::Instance)
        return found;

    Logme::Instance->IterateChannels([&](const std::string&, const Logme::ChannelPtr& channel)
    {
        if (!channel || found)
            return;

        int context = 0;
        const auto backend = channel->FindFirstBackend(Logme::FileBackend::TYPE_ID, context);
        const auto file = std::dynamic_pointer_cast<Logme::FileBackend>(backend);
        if (!file)
            return;

        const auto name = file->GetPathName();
        if (!name.empty())
            found = path_t(name);
    });
    return found;
}

}// namespace

void install()
{
    // a repeated translate() in the same process does not install a second filter
    static std::once_flag once;
    std::call_once(once, []
    {
#if defined(_WIN32)
        SetUnhandledExceptionFilter(on_exception);
#else
        struct sigaction action{};
        action.sa_sigaction = on_signal;
        action.sa_flags = SA_SIGINFO | SA_RESETHAND;
        sigemptyset(&action.sa_mask);
        // SA_RESETHAND: the handler fires once, then the signal action is SIG_DFL again
        sigaction(SIGSEGV, &action, nullptr);
        sigaction(SIGABRT, &action, nullptr);
        sigaction(SIGBUS, &action, nullptr);
        sigaction(SIGFPE, &action, nullptr);
        sigaction(SIGILL, &action, nullptr);
#endif
    });
}

path_ot arm_log()
{
    const auto opened = file_backend_path();
    if (!opened || !Logme::Instance)
        return {};

    // FileBackend opens galimatron.log with write denied to others.
    // The crash text goes to the neighboring galimatron.crash.log.
    path_t crash_log = *opened;
    crash_log.replace_filename(opened->stem().string() + ".crash" + opened->extension().string());

    const auto narrow = crash_log.string();
    if (!Logme::Instance->OpenCrashLog(narrow.c_str(), false))
        return {};

    Logme::Instance->SetCrashOutputMask(
        Logme::CRASH_OUTPUT_FILE | Logme::CRASH_OUTPUT_STDERR
    );
    return crash_log;
}

void arm_summary(const path_t& path)
{
    // path first, then the flag: the handler does not read the path while the flag is false
    summary_path = path;
    summary_ready.store(true, std::memory_order_release);
}

}// namespace galimatron::runtime::crash
