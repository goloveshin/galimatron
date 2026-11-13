#pragma once

// leaf_spy
#include <leaf_spy.hpp>

// Antlr4
#include <Exceptions.h>

// declaration of our exceptions
#include "galimatron/common_library/exception_declaration.hpp"


namespace galimatron::runtime
{

namespace print_exception_part
{

struct tabulation_t
{
    std::string section;
    std::string field;
    std::string warning;
};

// prints a domain or subdomain mark
//
// testing: no
//
struct visit_t
{
    std::ostringstream& ss;
    const tabulation_t& tabulation;
    // set to true if the pointer in visit() is present, otherwise nothing is done
    bool* is_visited;
};
template <typename Mark>
void visit_mark(const visit_t& args, const Mark* ptr, const char* part_name)
{
    if (ptr)
    {
        if (args.is_visited)
        {
            *args.is_visited = true;
        }
        args.ss << args.tabulation.section << part_name << std::endl;
    }
}

// testing: no
//
void visit(const visit_t&, const exception::e_what_info*);

// testing: no
//
void visit(const visit_t&, const exception::e_path_info*);

// testing: no
//
void visit(const visit_t&, const exception::e_aggregate_info*);

// testing: no
//
void visit(const visit_t&, const exception::e_version_info*);

// testing: no
//
void visit(const visit_t&, const exception::e_stacktrace_info*);

// testing: no
//
void visit(const visit_t&, const exception::details::io_operation_et*);

// testing: no
//
void visit(const visit_t&, const exception::details::validation_error_code_et*);

// testing: no
//
void visit(const visit_t&, const antlr4::ParseCancellationException*);

// testing: no
//
void visit(const visit_t&, const nlohmann::json::parse_error*);

// testing: no
//
void visit(const visit_t&, const leaf::diagnostic_info&);

// testing: no
//
void visit(const visit_t&, const leaf::diagnostic_details&);

// testing: no
//
void visit(const visit_t&, const std::exception*);

}// namespace print_exception_part

// short names for ::exception::domain and the subdomain namespaces
//
namespace leaf_mark
{
namespace domain = ::exception::domain;
namespace rt = ::exception::subdomain::runtime_library;
namespace tr = ::exception::subdomain::translator_library;
namespace details = ::exception::details;
}

// names the leaf slot types and prints them
//
// testing: no
//
struct spy_handler_t
{
    std::ostringstream& ss;

    void operator()(
        const leaf_mark::domain::executable*,
        const leaf_mark::domain::executable_library*,
        const leaf_mark::domain::runtime_library*,
        const leaf_mark::domain::translator_library*,
        const leaf_mark::rt::filesystem*,
        const leaf_mark::rt::filesystem_decorator*,
        const leaf_mark::rt::galimatron_config*,
        const leaf_mark::rt::translate_summary_xml*,
        const leaf_mark::rt::translate*,
        const leaf_mark::tr::parser*,
        const leaf_mark::tr::translator*,
        const leaf_mark::tr::wizard*,
        const leaf_mark::tr::builder*,
        const leaf_mark::tr::cpp2post_cpp*,
        const leaf_mark::tr::gal2cpp*,
        const leaf_mark::tr::holder*,
        const leaf_mark::tr::cpp2gal*,
        const leaf_mark::tr::cpp2xml*,
        const leaf_mark::tr::xml2cpp*,
        const leaf_mark::tr::template_engine*,
        const leaf_mark::details::io_operation_et*,
        const leaf_mark::details::validation_error_code_et*,
        const exception::e_what_info*,
        const exception::e_path_info*,
        const exception::e_aggregate_info*,
        const exception::e_version_info*,
        const exception::e_stacktrace_info*
    ) const
    {
    }

    // print our exceptions
    //
    template <typename Context>
    void visit(
        Context& ctx,
        const leaf::error_id id,
        std::exception* exception
    ) const
    {
        namespace pep = print_exception_part;
        using namespace leaf_mark;

        const auto tabulation = pep::tabulation_t("  ", "    ", "  ");

        bool is_any_of_domain = false;

        auto visit_args = pep::visit_t(ss, tabulation, &is_any_of_domain);

        ss << "Exception details:\n";

        // print std::exception first
        pep::visit(visit_args, exception);

        {
            scope_guard_t guard = scope_guard_t
            {
                [&]{ visit_args.is_visited = &is_any_of_domain; },
                [&]{ visit_args.is_visited = nullptr; }
            };

            pep::visit_mark(visit_args, ctx.template get<domain::executable>(id), "domain: executable");
            pep::visit_mark(visit_args, ctx.template get<domain::executable_library>(id), "domain: executable_library");
            pep::visit_mark(visit_args, ctx.template get<domain::runtime_library>(id), "domain: runtime_library");
            pep::visit_mark(visit_args, ctx.template get<domain::translator_library>(id), "domain: translator_library");
            pep::visit_mark(visit_args, ctx.template get<rt::filesystem>(id), "subdomain: runtime_library::filesystem");
            pep::visit_mark(visit_args, ctx.template get<rt::filesystem_decorator>(id), "subdomain: runtime_library::filesystem_decorator");
            pep::visit_mark(visit_args, ctx.template get<rt::galimatron_config>(id), "subdomain: runtime_library::galimatron_config");
            pep::visit_mark(visit_args, ctx.template get<rt::translate_summary_xml>(id), "subdomain: runtime_library::translate_summary_xml");
            pep::visit_mark(visit_args, ctx.template get<rt::translate>(id), "subdomain: runtime_library::translate");
            pep::visit_mark(visit_args, ctx.template get<tr::parser>(id), "subdomain: translator_library::parser");
            pep::visit_mark(visit_args, ctx.template get<tr::translator>(id), "subdomain: translator_library::translator");
            pep::visit_mark(visit_args, ctx.template get<tr::wizard>(id), "subdomain: translator_library::wizard");
            pep::visit_mark(visit_args, ctx.template get<tr::builder>(id), "subdomain: translator_library::builder");
            pep::visit_mark(visit_args, ctx.template get<tr::cpp2post_cpp>(id), "subdomain: translator_library::cpp2post_cpp");
            pep::visit_mark(visit_args, ctx.template get<tr::gal2cpp>(id), "subdomain: translator_library::gal2cpp");
            pep::visit_mark(visit_args, ctx.template get<tr::holder>(id), "subdomain: translator_library::holder");
            pep::visit_mark(visit_args, ctx.template get<tr::cpp2gal>(id), "subdomain: translator_library::cpp2gal");
            pep::visit_mark(visit_args, ctx.template get<tr::cpp2xml>(id), "subdomain: translator_library::cpp2xml");
            pep::visit_mark(visit_args, ctx.template get<tr::xml2cpp>(id), "subdomain: translator_library::xml2cpp");
            pep::visit_mark(visit_args, ctx.template get<tr::template_engine>(id), "subdomain: translator_library::template_engine");
            pep::visit(visit_args, dynamic_cast<const antlr4::ParseCancellationException*>(exception));
            pep::visit(visit_args, dynamic_cast<const nlohmann::json::parse_error*>(exception));
        }

        pep::visit(visit_args, ctx.template get<details::io_operation_et>(id));
        pep::visit(visit_args, ctx.template get<details::validation_error_code_et>(id));
        pep::visit(visit_args, ctx.template get<exception::e_what_info>(id));
        pep::visit(visit_args, ctx.template get<exception::e_path_info>(id));
        pep::visit(visit_args, ctx.template get<exception::e_aggregate_info>(id));
        pep::visit(visit_args, ctx.template get<exception::e_version_info>(id));
        pep::visit(visit_args, ctx.template get<exception::e_stacktrace_info>(id));

        if (!is_any_of_domain)
        {
            ss << tabulation.warning
                << "WARNING: NO DOMAIN-SPECIFIC EXCEPTION CATEGORY MATCHED\n";
        }

        pep::visit(
            visit_args,
            leaf::detail::diagnostic_info_(
                leaf::error_info(
                    id,
                    exception,
                    ctx.template get<leaf::e_source_location>(id)
                ),
                ctx
            )
        );
    }
};

// testing: no
//
struct leaf_spy_args_t
{
    using notify_printed_ft = std::function<void (const std::string)>;

    ::leaf_spy::leaf_spy_args_t global_args;

    // return the printed exception info, if any
    const notify_printed_ft& notify_printed;
};
template <typename ... DirectHandlers>
void try_catch(
    const leaf_spy_args_t& args,
    DirectHandlers&& ... direct_handlers
)
{
    std::ostringstream ss;

    auto notify_printed_on_exit = scope_guard_t
    {
        []{},
        [&]{
            if (const auto str = ss.str(); !str.empty())
            {
                args.notify_printed(str);
            }
        }
    };

    ::leaf_spy::try_catch(
        args.global_args,
        spy_handler_t{ ss },
        std::forward<DirectHandlers>(direct_handlers)...
    );
}

}// namespace galimatron::runtime
