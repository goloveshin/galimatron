// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/wizard/tags.hpp"
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/exception_augment.hpp"
#include "galimatron/translator_library/capture_or_rethrow.hpp"

// global
#include "global/common_library/utils.hpp"

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

[[noreturn]] void fail(
    const exception::details::validation_error_code_et code,
    std::string what,
    const exception::error_where_t& where
)
{
    // path, aggregate, and version are loaded inside the capture.
    // a load in the caller attaches to captured_leaf_exception, and try_catch unpacks it without them
    capture_or_rethrow([&]
    {
        const auto location = exception::augment_where(where);
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::translator{},
            code,
            exception::e_what_info{ std::move(what) }
        );
    });
}

// a name already chosen in this list, compared without case
//
bool name_already_listed(const strings_t& names, const name_t& name)
{
    return std::ranges::any_of(names, [&](const name_t& kept)
    {
        return boost::iequals(kept, name);
    });
}

std::string join_indices(const indices_t& indices)
{
    auto range =
        indices
        | ranges::views::transform([](const index_t index){ return std::to_string(index); })
        | ranges::to<std::vector>()
        ;
    return boost::join(range, ",");
}

bool index_is_listed(const indices_t& indices, const index_t index)
{
    return std::ranges::find(indices, index) != indices.end();
}

// empty, or 0, 1, 2, ... with no gaps
//
bool indexes_stay_continuous(const indices_t& indices)
{
    if (indices.empty())
    {
        return true;
    }
    if (indices.front() != 0)
    {
        return false;
    }
    for (index_t position = 1; position < indices.size(); ++position)
    {
        if (indices[position] != indices[position - 1] + 1)
        {
            return false;
        }
    }
    return true;
}

bool is_deleted(const version_status_oet& status)
{
    return status == version_status_et::DELETED;
}

// the definition consists only of delete; — a historical version deletion
//
bool definition_is_only_version_delete(
    const state::holder::language_part_t& language,
    const state::vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    const auto vd = language.versions().try_definition(vd_aggregate, index);
    if (!vd)
    {
        return false;
    }
    return language.is_only_version_delete(*vd);
}

const state::wizard_statement_t& wizard_statement(const state::holder::language_part_t& language_part)
{
    const auto statements = language_part.wizard_statements();
    return std::get<state::wizard_statement_t>(language_part.graph()[statements.front()].entity);
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
