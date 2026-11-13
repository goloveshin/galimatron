#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"


namespace galimatron::translator::state::holder
{

// aggregated static label: name + values
struct aggregated_label_static_t
{
    name_t name;
    const label_static_t& label_static;
};
using aggregated_label_static_rt = std::reference_wrapper<const aggregated_label_static_t>;
using aggregated_labels_static_t = std::deque<aggregated_label_static_rt>;
using aggregated_labels_static_ot = std::optional<aggregated_labels_static_t>;

}// namespace galimatron::translator::state::holder
