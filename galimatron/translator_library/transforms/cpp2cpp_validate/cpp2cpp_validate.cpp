// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/capture_or_rethrow.hpp"

// Taskflow
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <taskflow/taskflow.hpp>

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2cpp_validate
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


namespace
{

// compilation checks the language, including that a tag has every label it uses by the time it is declared.
// stops at the first error. no root aggregate — nothing to compile.
holder::compiled_part_sptr_ot compile_for_validation(const holder::language_part_sptr_t& language)
{
    if (!language || !language->aggregates().has_root())
    {
        return {};
    }

    holder::compiled_part_sptr_ot compiled;

    // empty result: try_capture_all for void does not drop the context on success
    capture_or_rethrow([&]() -> leaf::result<void>
    {
        compiled = transforms::cpp2post_cpp::translate(language);
        return {};
    });
    return compiled;
}

}// namespace

// the checks that run after gal2cpp, and again after a user xslt.
// a user xslt does not rewrite a hash. a hash that no longer matches fails here.
// compilation stops at the first error. no language — nothing is checked.
// the returned compile is empty when there is no root aggregate
//
// testing: yes (integration)
//
holder::compiled_part_sptr_ot validate_language(
    const holder::language_part_sptr_t& language,
    tf::Subflow& subflow
)
{
    if (!language)
    {
        return {};
    }

    validate_memo_t memo;
    cpp2cpp_validate_t validate(*language, memo);
    validate.build(subflow);
    subflow.join();
    return compile_for_validation(language);
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
