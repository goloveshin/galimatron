#pragma once

// galimatron
#include "galimatron/tests/forward.hpp"
#include "galimatron/runtime_library/leaf_spy/leaf_spy.hpp"


// REGISTER_EXCEPTION_TRANSLATOR cannot be used for leaf exceptions,
// so we use wrappers for printing to the console

// CHECK_THROW_WITH(print_leaf_exception([]{ function under test(); }));
// CHECK_NOTHROW(print_nothrow([]{ function under test(); }));
//
// testing: no
//
template <typename ... DirectHandlers>
void print_leaf_exception(
    // function under test
    const std::function<void ()>& check_func,
    // print info to the console (test debugging)
    const bool is_print_to_cout = false,
    // user catch functions
    DirectHandlers&& ... direct_handlers
)
{
    galimatron::runtime::try_catch(
        {
            .global_args = {
                .job_that_may_throw = [&]
                {
                    check_func();
                },
                .notify_that_job_executed_with_throw = []{},
                .notify_that_job_executed_without_throw = []{},
                .is_throw_out = true
            },
            .notify_printed = [&](const std::string& info)
            {
                if (is_print_to_cout)
                {
                    std::cout << info << std::endl;
                }
            }
        },
        std::forward<DirectHandlers>(direct_handlers)...
    );
}

namespace galimatron
{

// builtin steps of a translator slice
//
struct translate_slice_t
{
    bool gal2cpp;
    bool cpp2cpp_validate;
    bool cpp2cpp_wizard;
    bool cpp2xml;
    bool xml2cpp;
    bool cpp2gal;
    bool cpp2post_cpp;
    bool post_cpp2post_xml;

    // forget stored tag and ordering text before cpp2gal, so the printers use the default layout
    bool drop_tag_user_formatting = false;
};

// Product: full integration test
// log.cfg and gal.cfg from the case directory; exceptions leave translate()
//
// testing: no
//
void translate_production(const path_t& case_root);

// Translator slice: partial integration test
// the flags choose the builtin steps
// .gal lands in output-gal-dir, xml in intermediate-xml-dir
//
// testing: no
//
void translate_slice(const path_t& case_root, const translate_slice_t& args);

// copies one file onto destination, creating the parent directories.
// a product test stages a fixture this way, then calls translate_production
//
// testing: no
//
void stage_file(const path_t& source, const path_t& destination);

// replaces destination with a copy of the source directory.
// the source stays in place, so the next run can stage it again
//
// testing: no
//
void stage_directory(const path_t& source, const path_t& destination);

}// namespace galimatron
