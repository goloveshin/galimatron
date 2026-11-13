// galimatron
#include "galimatron/runtime_library/leaf_spy/leaf_spy.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::runtime::print_exception_part
{

using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt);
}

void visit_helper(
    const visit_t& args,
    const auto* ptr,
    const std::string& part_name,
    const auto& visit_func
)
{
    if (ptr)
    {
        if (args.is_visited)
        {
            *args.is_visited = true;
        }

        args.ss << args.tabulation.section << part_name << ":\n";

        visit_func(*ptr);
    }
};


void visit(const visit_t& args, const exception::e_what_info* ptr)
{
    visit_helper(args, ptr, "what info", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << "message: " << ref.what << '\n';
    });
}

void visit(const visit_t& args, const exception::e_path_info* ptr)
{
    if (!ptr || !ptr->path)
    {
        return;
    }
    visit_helper(args, ptr, "path info", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << "path: " << report_path(
            *ref.path,
            ref.committed_report.value_or(false),
            ref.config_dir.value_or(path_t{})
        ) << '\n';
    });
}

void visit(const visit_t& args, const exception::e_aggregate_info* ptr)
{
    visit_helper(args, ptr, "aggregate info", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << "name: " << ref.name << '\n';
    });
}

void visit(const visit_t& args, const exception::e_version_info* ptr)
{
    visit_helper(args, ptr, "version info", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << "version: " << ref.version << '\n';
    });
}

void visit(const visit_t& args, const exception::e_stacktrace_info* ptr)
{
    visit_helper(args, ptr, "stacktrace info", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << ref.stacktrace << '\n';
    });
}

void visit(const visit_t& args, const exception::details::io_operation_et* ptr)
{
    visit_helper(args, ptr, "io operation", [&](const auto& ref)
    {
        args.ss << args.tabulation.field
            << "operation: " << magic_enum::enum_name(ref) << '\n';
    });
}

void visit(const visit_t& args, const exception::details::validation_error_code_et* ptr)
{
    visit_helper(args, ptr, "validation", [&](const auto& ref)
    {
        args.ss << args.tabulation.field
            << "what: " << magic_enum::enum_name(ref) << '\n';
    });
}

void visit(const visit_t& args, const antlr4::ParseCancellationException* ptr)
{
    visit_helper(args, ptr, "type: Antlr ParseCancellationException", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << "message: " << ref.what() << '\n';
    });
}

void visit(const visit_t& args, const nlohmann::json::parse_error* ptr)
{
    visit_helper(args, ptr, "type: JSON parse error", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << "byte: " << ref.byte << '\n';
    });
}

void visit(const visit_t& args, const leaf::diagnostic_info& info)
{
    visit_helper(args, &info, "Boost.LEAF diagnostic info", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << ref << '\n';
    });
}

void visit(const visit_t& args, const leaf::diagnostic_details& info)
{
    visit_helper(args, &info, "Boost.LEAF diagnostic details", [&](const auto& ref)
    {
        args.ss << args.tabulation.field << ref << '\n';
    });
}

void visit(const visit_t& args, const std::exception* ptr)
{
    if (!ptr)
    {
        return;
    }

    args.ss << args.tabulation.section << "type: std::exception:\n";
    args.ss << args.tabulation.field << "message: " << ptr->what() << '\n';
}

}// namespace galimatron::runtime::print_exception_part
