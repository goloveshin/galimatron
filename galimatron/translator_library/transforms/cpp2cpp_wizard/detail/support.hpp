#pragma once

// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

wizard_statement_t& wizard_statement(holder::language_part_t& state);

// the mode stored on a version body. an absent body inherits
//
inline tag_mode_et mode_of_definition(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_definition
)
{
    const auto& entity = state.graph()[vd_definition].entity;
    if (const auto* definition = std::get_if<root_aggregate_version_definition_t>(&entity))
    {
        return definition->mode;
    }
    if (const auto* definition = std::get_if<aggregate_version_definition_t>(&entity))
    {
        return definition->mode;
    }
    return tag_mode_et::INHERIT;
}

inline tag_mode_et mode_of_version(
    const holder::language_part_t& state,
    const vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    const auto vd = state.versions().try_definition(vd_aggregate, index);
    if (!vd)
    {
        return tag_mode_et::INHERIT;
    }
    return mode_of_definition(state, *vd);
}

// plain create version keeps local and own. once, local, and own on the command force that word.
// local on the root is written once. own on the root is written unmarked
//
inline tag_mode_et mode_for_new_version(
    const bool is_root,
    const tag_mode_et forced,
    const tag_mode_et previous
)
{
    if (forced == tag_mode_et::LOCAL)
    {
        return is_root ? tag_mode_et::ONCE : tag_mode_et::LOCAL;
    }
    if (forced == tag_mode_et::ONCE)
    {
        return tag_mode_et::ONCE;
    }
    if (forced == tag_mode_et::OWN)
    {
        return is_root ? tag_mode_et::INHERIT : tag_mode_et::OWN;
    }
    if (!is_root && previous == tag_mode_et::LOCAL)
    {
        return tag_mode_et::LOCAL;
    }
    if (!is_root && previous == tag_mode_et::OWN)
    {
        return tag_mode_et::OWN;
    }
    return tag_mode_et::INHERIT;
}

// the next version on the root and on every aggregate, the same way wizard create version does.
// copy_local_body is false when continue aggregate copies the body itself.
// returns the new index
//
index_t add_next_version(
    holder::language_part_t& state,
    state::builder::builder_t& builder,
    tag_mode_et forced = tag_mode_et::INHERIT,
    bool copy_local_body = true
);

// hash of one version's important tags into each live aggregate's declaration.
// the root declaration stores the hash of those hashes
//
void write_version_hash(
    holder::language_part_t& language,
    const index_t index
);

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
