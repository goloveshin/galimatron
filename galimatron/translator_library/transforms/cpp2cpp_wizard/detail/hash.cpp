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
void set_declaration_hash(
    state::holder::language_part_t::graph_t& graph,
    const state::vertex_descriptor_t vd,
    const std::string& hash
)
{
    auto& vertex = graph[vd];
    // the stored source text still shows the declaration before this hash
    vertex.user_formatting.reset();

    if (auto* declaration = std::get_if<state::root_aggregate_version_declaration_t>(&vertex.entity))
    {
        declaration->hash = hash;
        return;
    }
    if (auto* declaration = std::get_if<state::aggregate_version_declaration_t>(&vertex.entity))
    {
        declaration->hash = hash;
    }
}

void clear_version_hash(
    state::holder::language_part_t::graph_t& graph,
    const state::vertex_descriptor_t vd
)
{
    auto& vertex = graph[vd];
    // the stored source text still shows the declaration before the hash was removed
    vertex.user_formatting.reset();

    if (auto* declaration = std::get_if<state::root_aggregate_version_declaration_t>(&vertex.entity))
    {
        declaration->hash.reset();
    }
    else if (auto* declaration = std::get_if<state::aggregate_version_declaration_t>(&vertex.entity))
    {
        declaration->hash.reset();
    }
}

struct version_hash_row_t
{
    bool is_root = false;
    std::string hash;
    state::vertex_descriptor_t vd_declaration = state::vertex_descriptor_not_present;
};

// live aggregates only. a deleted declaration is not part of the hash
//
std::vector<version_hash_row_t> live_version_hash_rows(
    state::holder::language_part_t& language,
    const index_t index
)
{
    auto& graph = language.graph();
    std::vector<version_hash_row_t> rows;
    for (const auto& record : language.aggregates())
    {
        const auto declaration = language.versions().try_declaration(record.vd, index);
        if (!declaration)
        {
            continue;
        }
        if (language.versions().status(record.vd, index) == version_status_et::DELETED)
        {
            continue;
        }

        rows.push_back({
            .is_root = record.is_root,
            .hash = content_hash::important_tags_hash(
                graph,
                language.versions().try_definition(record.vd, index)
            ),
            .vd_declaration = *declaration
        });
    }

    return rows;
}

void clear_deleted_version_hashes(
    state::holder::language_part_t& language,
    const index_t index
)
{
    auto& graph = language.graph();
    for (const auto& record : language.aggregates())
    {
        if (language.versions().status(record.vd, index) != version_status_et::DELETED)
        {
            continue;
        }
        const auto declaration = language.versions().try_declaration(record.vd, index);
        if (!declaration)
        {
            continue;
        }
        if (std::get<state::aggregate_version_declaration_t>(graph[*declaration].entity).hash)
        {
            clear_version_hash(graph, *declaration);
        }
    }
}

// copies the declaration hash, comment, and status into the version container
//
void remember_stored_fields(
    state::holder::language_part_t& language,
    const state::vertex_descriptor_t vd_aggregate,
    const index_t index
)
{
    const auto vd = language.versions().try_declaration(vd_aggregate, index);
    if (!vd)
    {
        return;
    }

    const auto& entity = language.graph()[*vd].entity;
    const auto* root = std::get_if<state::root_aggregate_version_declaration_t>(&entity);
    const auto* aggregate = std::get_if<state::aggregate_version_declaration_t>(&entity);
    if (!root && !aggregate)
    {
        return;
    }

    language.versions().set_declaration_fields(
        vd_aggregate,
        index,
        root ? root->hash : aggregate->hash,
        root ? root->comment : aggregate->comment,
        root ? root->status : aggregate->status
    );
}

// removes the hash of one version from every aggregate declaration
//
void clear_hashes_of_version(
    state::holder::language_part_t& language,
    const index_t index
)
{
    auto& graph = language.graph();
    for (const auto& record : language.aggregates())
    {
        if (const auto declaration = language.versions().try_declaration(record.vd, index))
        {
            clear_version_hash(graph, *declaration);
        }
        remember_stored_fields(language, record.vd, index);
    }
}
}// namespace

// hash of one version's important tags into each live aggregate's declaration.
// the root declaration stores the hash of those hashes, sorted, so only the content is part of it.
// a deleted declaration stays without a hash
//
void write_version_hash(
    state::holder::language_part_t& language,
    const index_t index
)
{
    auto& graph = language.graph();
    clear_deleted_version_hashes(language, index);
    const auto rows = live_version_hash_rows(language, index);

    std::vector<std::string> hashes;
    for (const auto& row : rows)
    {
        if (!row.is_root)
        {
            set_declaration_hash(graph, row.vd_declaration, row.hash);
        }
        hashes.push_back(row.hash);
    }

    const auto root_hash = content_hash::hash_of_hashes(hashes);
    for (const auto& row : rows)
    {
        if (row.is_root)
        {
            set_declaration_hash(graph, row.vd_declaration, root_hash);
        }
    }

    // the seal closes this version from last_hashed. the stored hash is left as written
    for (const auto& record : language.aggregates())
    {
        remember_stored_fields(language, record.vd, index);
    }
}

// hash of each listed version's important tags into each live aggregate's declaration.
// no version means the last version. the root declaration stores the hash of those hashes.
// aggregate names are not part of it. a deleted declaration stays without a hash
//
void cpp2cpp_wizard_t::apply_wizard_create_hash()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_HASH)
    {
        return;
    }

    for (const auto index : validate_->hash_version_indexes())
    {
        write_version_hash(*language_state_, index);
    }
}

// removes the hash of each listed version from every aggregate.
// no version means the last version
//
void cpp2cpp_wizard_t::apply_wizard_delete_hash()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::DELETE_HASH)
    {
        return;
    }

    for (const auto index : validate_->hash_version_indexes())
    {
        clear_hashes_of_version(*language_state_, index);
    }
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
