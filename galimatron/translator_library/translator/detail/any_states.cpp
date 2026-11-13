// galimatron
#include "galimatron/translator_library/translator/detail/any_states.hpp"


namespace galimatron::translator::translator
{

std::any& any_states_t::state_slot(const state_slot_et which)
{
    return slot[static_cast<std::size_t>(which)];
}

layout::table_t& any_states_t::layout_table()
{
    auto& held = state_slot(state_slot_et::LAYOUT_TABLE);
    if (!held.has_value())
    {
        held = layout::table_t{};
    }
    return std::any_cast<layout::table_t&>(held);
}

}// namespace galimatron::translator::translator
