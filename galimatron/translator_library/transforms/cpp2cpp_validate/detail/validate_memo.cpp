// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"

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


bool_ot validate_memo_t::get_is_root_aggregate_single() const
{
    std::shared_lock lock{ mutex_ };
    return is_root_aggregate_single_;
}

void validate_memo_t::store_is_root_aggregate_single(const bool value)
{
    std::unique_lock lock{ mutex_ };
    is_root_aggregate_single_ = value;
}

bool_ot validate_memo_t::get_is_root_aggregate_version_declarations_from_zero_continuous() const
{
    std::shared_lock lock{ mutex_ };
    return is_root_aggregate_version_declarations_from_zero_continuous_;
}

void validate_memo_t::store_is_root_aggregate_version_declarations_from_zero_continuous(const bool value)
{
    std::unique_lock lock{ mutex_ };
    is_root_aggregate_version_declarations_from_zero_continuous_ = value;
}

bool_ot validate_memo_t::get_is_root_aggregate_version_definitions_repeat_declarations() const
{
    std::shared_lock lock{ mutex_ };
    return is_root_aggregate_version_definitions_repeat_declarations_;
}

void validate_memo_t::store_is_root_aggregate_version_definitions_repeat_declarations(const bool value)
{
    std::unique_lock lock{ mutex_ };
    is_root_aggregate_version_definitions_repeat_declarations_ = value;
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
