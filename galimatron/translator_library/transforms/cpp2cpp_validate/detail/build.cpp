// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/wizard/tags.hpp"
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// Boost.LEAF
#include <boost/leaf/handle_errors.hpp>

// global
#include "global/common_library/utils.hpp"

// leaf_spy
#include "captured_leaf_exception.hpp"

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

void cpp2cpp_validate_t::build(tf::Subflow& subflow)
{
    subflow.retain(true);

    // create root aggregate is applied after validating; the root aggregate does not exist yet.
    // the command's own checks are called in the command and use this same cpp2cpp_validate_t
    const bool skip_root_checks = skips_root_aggregate_checks();

    auto is_root_aggregate_single = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_root_aggregate_single();
        }
    }).name("is root aggregate single");

    auto is_root_aggregate_version_declarations_from_zero_continuous = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_root_aggregate_version_declarations_from_zero_continuous();
        }
    }).name("is root aggregate version declarations from zero continuous");

    auto is_root_aggregate_version_definitions_repeat_declarations = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_root_aggregate_version_definitions_repeat_declarations();
        }
    }).name("is root aggregate version definitions repeat declarations");

    auto is_aggregate_version_declarations_match_root = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_aggregate_version_declarations_match_root();
        }
    }).name("is aggregate version declarations match root");

    auto is_aggregate_version_declarations_deleted_continuous = subflow.emplace([this]
    {
        this->is_aggregate_version_declarations_deleted_continuous();
    }).name("is aggregate version declarations deleted continuous");

    auto is_aggregate_version_definitions_not_deleted = subflow.emplace([this]
    {
        this->is_aggregate_version_definitions_not_deleted();
    }).name("is aggregate version definitions not deleted");

    auto is_aggregate_version_definitions_repeat_declarations = subflow.emplace([this]
    {
        this->is_aggregate_version_definitions_repeat_declarations();
    }).name("is aggregate version definitions repeat declarations");

    auto is_anonymous_versions_owned = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_anonymous_versions_owned();
        }
    }).name("is anonymous versions owned");

    auto is_version_hashes_match = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_version_hashes_match();
        }
    }).name("is version hashes match");

    auto is_aggregate_names_unique = subflow.emplace([this]
    {
        this->is_aggregate_names_unique();
    }).name("is aggregate names unique");

    auto is_aggregate_indices_unique_continuous = subflow.emplace([this]
    {
        this->is_aggregate_indices_unique_continuous();
    }).name("is aggregate indices unique continuous");

    auto is_version_hashes_present = subflow.emplace([this, skip_root_checks]
    {
        if (!skip_root_checks)
        {
            this->is_version_hashes_present();
        }
    }).name("is version hashes present");

    // first a single root aggregate, then both version checks in parallel.
    // aggregate declarations are checked against the already checked root declarations
    is_root_aggregate_single.precede(
        is_root_aggregate_version_declarations_from_zero_continuous,
        is_root_aggregate_version_definitions_repeat_declarations,
        is_aggregate_version_declarations_deleted_continuous,
        is_aggregate_version_definitions_not_deleted,
        is_aggregate_version_definitions_repeat_declarations,
        is_anonymous_versions_owned,
        is_aggregate_names_unique,
        is_aggregate_indices_unique_continuous
    );

    is_root_aggregate_version_declarations_from_zero_continuous.precede(
        is_aggregate_version_declarations_match_root
    );

    is_aggregate_version_declarations_match_root.precede(is_version_hashes_match, is_version_hashes_present);
    is_root_aggregate_version_definitions_repeat_declarations.precede(is_version_hashes_match, is_version_hashes_present);
    is_aggregate_version_declarations_deleted_continuous.precede(is_version_hashes_match, is_version_hashes_present);
    is_aggregate_version_definitions_not_deleted.precede(is_version_hashes_match, is_version_hashes_present);
    is_aggregate_version_definitions_repeat_declarations.precede(is_version_hashes_match, is_version_hashes_present);
    is_anonymous_versions_owned.precede(is_version_hashes_match, is_version_hashes_present);
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
