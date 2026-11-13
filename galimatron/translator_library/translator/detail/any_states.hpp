#pragma once

// galimatron
#include "galimatron/translator_library/transforms/layout_table.hpp"

// magic_enum
#include <magic_enum/magic_enum.hpp>

// std
#include <any>
#include <array>


namespace galimatron::translator::translator
{

enum class state_slot_et
{
    SAXON_SESSION,
    LAYOUT_TABLE
};

// named slots shared by the jobs of one translation
//
struct any_states_t
{
    std::array<std::any, magic_enum::enum_count<state_slot_et>()> slot;

    // one slot of the bag
    //
    std::any& state_slot(state_slot_et);

    // the layout table for this translation. an empty slot is created empty.
    // throws std::bad_any_cast when the slot holds another type
    //
    layout::table_t& layout_table();
};

}// namespace galimatron::translator::translator
