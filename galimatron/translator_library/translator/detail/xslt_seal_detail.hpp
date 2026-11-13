#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"


namespace galimatron::translator::translator
{

using namespace galimatron::translator::state;

enum class change_kind_et
{
    REASSIGN,
    DELETE_TAG
};

struct change_t
{
    change_kind_et kind;
    name_t name;
    vertex_descriptor_ot vd_tag;
};

struct aggregate_edit_t
{
    name_t name;
    std::vector<change_t> changes;
};

const holder::aggregate_container::detail::record_t* find_aggregate(
    const holder::aggregate_container::container_t& aggregates,
    const name_t& name
);

[[noreturn]] void throw_seal(const std::string& what);

index_ot last_sealed_index(const holder::language_part_t& before);

void reject_sealed_shape(
    const holder::language_part_t& before,
    const holder::language_part_t& after,
    const index_t last_sealed
);

bool sealed_history_differs(
    const holder::language_part_t& before,
    const holder::language_part_t& after,
    const index_t last_sealed
);

vertex_descriptor_t declarations_node(
    builder::builder_t& builder,
    holder::language_part_t& edited,
    const vertex_descriptor_t vd_aggregate,
    const bool is_root
);

vertex_descriptor_t definitions_node(
    builder::builder_t& builder,
    holder::language_part_t& edited,
    const vertex_descriptor_t vd_aggregate,
    const bool is_root
);

void restore_sealed_history(
    const holder::language_part_t& before,
    holder::language_part_t& after,
    const index_t last_sealed
);

// every sealed edit is a wizard copy the xslt is allowed to finish
//
bool only_wizard_dynamic_copies(
    const holder::language_part_t& language,
    const std::vector<aggregate_edit_t>& edits,
    const index_t version_index
);

std::vector<aggregate_edit_t> effective_edits(
    const holder::language_part_sptr_t& before,
    const holder::language_part_sptr_t& after,
    const index_t last_sealed
);

index_t destination_version(holder::language_part_t& edited, const index_t last_sealed);

void write_edits(
    holder::language_part_t& edited,
    const index_t version_index,
    const std::vector<aggregate_edit_t>& edits
);

}// namespace galimatron::translator::translator
