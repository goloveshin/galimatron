// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// global
#include "global/common_library/utils.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

namespace
{
void remove_version(
    holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    if (const auto vd = state.versions().try_definition(vd_aggregate, index))
    {
        state.detach_children(*vd);
        state.detach_vertex(state.graph()[*vd].vd_parent, *vd);
    }
    if (const auto vd = state.versions().try_declaration(vd_aggregate, index))
    {
        state.graph()[*vd].user_formatting.reset();
        state.detach_vertex(state.graph()[*vd].vd_parent, *vd);
    }
    state.versions().forget(vd_aggregate, index);
}
}// namespace

// the named versions are removed from every aggregate. no version means the last version.
// the versions that remain run from 0 without gaps, so a following create version continues that sequence
//
void cpp2cpp_wizard_t::apply_wizard_delete_version()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::DELETE_VERSION)
    {
        return;
    }

    auto indexes = validate_->delete_version_indexes();
    indexes |= ranges::actions::sort | ranges::actions::unique;

    const auto aggregates =
        language_state_->aggregates()
        | ranges::views::transform([](const auto& record){ return record.vd; })
        | ranges::to_vector
        ;
    for (const auto index : indexes)
    {
        for (const auto vd_aggregate : aggregates)
        {
            remove_version(*language_state_, vd_aggregate, index);
        }
    }
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
