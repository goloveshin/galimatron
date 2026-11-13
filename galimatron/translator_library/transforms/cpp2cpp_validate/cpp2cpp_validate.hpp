#pragma once

// galimatron
#include "galimatron/common_library/type_declaration.hpp"
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/wizard/data.hpp"


namespace tf
{
class Subflow;
}

namespace galimatron::translator::transforms::cpp2cpp_validate
{

using namespace galimatron::translator::state;

// memoization of check results; reads and writes are concurrent
class validate_memo_t
{
public:
    validate_memo_t() = default;
    validate_memo_t(const validate_memo_t&) = delete;
    validate_memo_t(validate_memo_t&&) = delete;
    validate_memo_t& operator=(const validate_memo_t&) = delete;
    validate_memo_t& operator=(validate_memo_t&&) = delete;

public:
    bool_ot get_is_root_aggregate_single() const;
    void store_is_root_aggregate_single(bool);

    bool_ot get_is_root_aggregate_version_declarations_from_zero_continuous() const;
    void store_is_root_aggregate_version_declarations_from_zero_continuous(bool);

    bool_ot get_is_root_aggregate_version_definitions_repeat_declarations() const;
    void store_is_root_aggregate_version_definitions_repeat_declarations(bool);

private:
    mutable std::shared_mutex mutex_;

    bool_ot is_root_aggregate_single_;
    bool_ot is_root_aggregate_version_declarations_from_zero_continuous_;
    bool_ot is_root_aggregate_version_definitions_repeat_declarations_;
};

// the checks that run after gal2cpp, and again after a user xslt.
// a user xslt does not rewrite a hash. a hash that no longer matches fails here.
// compilation stops at the first error. no language — nothing is checked.
// the returned compile is empty when there is no root aggregate
//
// testing: yes (integration)
//
holder::compiled_part_sptr_ot validate_language(
    const holder::language_part_sptr_t&,
    tf::Subflow&
);

// check of the Language state
//
// testing: yes (unit), yes (integration)
//
class cpp2cpp_validate_t
{
public:
    cpp2cpp_validate_t(
        const state::holder::language_part_t&,
        validate_memo_t&
    );
    cpp2cpp_validate_t(const cpp2cpp_validate_t&) = delete;
    cpp2cpp_validate_t(cpp2cpp_validate_t&&) = delete;
    cpp2cpp_validate_t& operator=(const cpp2cpp_validate_t&) = delete;
    cpp2cpp_validate_t& operator=(cpp2cpp_validate_t&&) = delete;

public:
    // the root aggregate is exactly one, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    // and the list of files that contain it
    //
    bool is_root_aggregate_single();

    // root aggregate version declarations run from 0 without gaps,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_root_aggregate_version_declarations_from_zero_continuous();

    // root aggregate version definition indices repeat the live declaration indices,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_root_aggregate_version_definitions_repeat_declarations();

    // each aggregate's version declarations repeat the root declarations,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_aggregate_version_declarations_match_root();

    // deleted is a prefix, a suffix, or an interior span owned by the next live version's anonymous body,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_aggregate_version_declarations_deleted_continuous();

    // aggregate version definitions do not refer to a deleted declaration,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_aggregate_version_definitions_not_deleted();

    // aggregate version definitions repeat the declarations without deleted,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_aggregate_version_definitions_repeat_declarations();

    // aggregate names are unique, including the root.
    // no single root aggregate is left to is_root_aggregate_single.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_aggregate_names_unique();

    // non-root aggregate indices are 0 .. n-1 without gaps or duplicates,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_aggregate_indices_unique_continuous();

    // the stored hash of a version declaration matches the one computed the same way as wizard create hash.
    // a deleted declaration is left out. aggregate names are not part of the root hash.
    // checked per version and against the root hash. no hash — the declaration is not checked.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator.
    // the message names the aggregates of each differing version, then a version that differs only in the root.
    // a version that already names an aggregate does not name the root
    //
    bool is_version_hashes_match();

    // a version that stores a hash stores it on the root and on every aggregate that is not deleted.
    // a deleted declaration stays without a hash.
    // a version with no hashes is not checked.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    // and every declaration where the hash is absent
    //
    bool is_version_hashes_present();

    // among all commands there is exactly one wizard command, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_command_single();

    // a command inside a wizard batch is a create command, including create hash, delete tag, delete version, or collapse version,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_batch_command_create();

    // create tag, delete tag, create label, and create ordering write into the open version.
    // a chunk names the aggregate. a version definition does not.
    // ordering writes only into the root aggregate.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_version_statement();

    // checks of the current wizard command, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool may_execute_wizard_command();

    // versions named by create hash or delete hash.
    // no version means the last version of the root aggregate.
    // empty when that version does not exist.
    //
    indices_t hash_version_indexes();

    // versions named by delete version.
    // no version means the last version of the root aggregate.
    // empty when that version does not exist.
    //
    indices_t delete_version_indexes();

public:
    // fills the subflow with the checks. the caller joins.
    //
    void build(tf::Subflow&);

private:
    // root aggregate version declaration indices, from the snapshot
    //
    indices_t version_declaration_indices();

    // root aggregate version definition indices, from the snapshot
    //
    indices_t version_definition_indices();

    bool is_wizard_create_root_aggregate_context_chunk();
    bool is_wizard_create_root_aggregate_command_file();
    bool is_wizard_create_root_aggregate_chunk_absent();
    bool is_wizard_create_root_aggregate_root_aggregate_absent();

    bool wizard_create_root_aggregate_context_is_chunk(wizard::context_et) const;
    bool wizard_create_root_aggregate_root_aggregate_is_absent() const;

    bool is_wizard_create_aggregate_context_chunk();
    bool is_wizard_create_aggregate_root_aggregate_present();

    // create aggregate writes a definition of the last root version.
    // that version is open, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_create_aggregate_last_version_open();

    bool is_wizard_create_aggregate_command_file();
    bool is_wizard_create_aggregate_names_unique();
    bool is_wizard_create_aggregate_chunk_absent();

    // copy aggregate writes each new name from a source that already exists.
    // a source still present in a closed version is refused.
    //
    bool is_wizard_copy_aggregate_context_chunk();
    bool is_wizard_copy_aggregate_root_aggregate_present();
    bool is_wizard_copy_aggregate_source_present();
    bool is_wizard_copy_aggregate_not_root();
    bool is_wizard_copy_aggregate_names_unique();
    bool is_wizard_copy_aggregate_aggregates_absent();
    bool is_wizard_copy_aggregate_command_file();
    bool is_wizard_copy_aggregate_chunk_absent();
    bool is_wizard_copy_aggregate_source_open();

    // continue aggregate adds the next version, ends the source there, and continues each new name.
    // the source's last version is live.
    //
    bool is_wizard_continue_aggregate_context_chunk();
    bool is_wizard_continue_aggregate_root_aggregate_present();
    bool is_wizard_continue_aggregate_source_present();
    bool is_wizard_continue_aggregate_not_root();
    bool is_wizard_continue_aggregate_names_unique();
    bool is_wizard_continue_aggregate_aggregates_absent();
    bool is_wizard_continue_aggregate_command_file();
    bool is_wizard_continue_aggregate_chunk_absent();
    bool is_wizard_continue_aggregate_last_version_live();

    bool is_wizard_delete_aggregate_context_chunk();
    bool is_wizard_delete_aggregate_root_aggregate_present();
    bool is_wizard_delete_aggregate_names_unique();
    bool is_wizard_delete_aggregate_aggregates_exist();
    bool is_wizard_delete_aggregate_not_root();
    bool is_wizard_delete_aggregate_last_version_live();
    bool is_wizard_delete_aggregate_last_version_open();
    bool is_wizard_delete_aggregate_version_definition_present();

    // physically removes the chunk. an aggregate still present in a closed version is refused,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_aggregate_physically_closed();

    // physically removes the chunk. a version whose ordering names the aggregate is refused,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_aggregate_physically_not_ordered();

    // create version runs in an empty chunk,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_create_version_context();

    // create version needs the root aggregate, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_create_version_root_present();

    // delete version runs in an empty chunk,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_version_context();

    // delete version needs the root aggregate, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_version_root_present();

    // each listed version is present in the root aggregate declarations.
    // no version means the last version, and that version must exist.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_version_present();

    // each listed version is open. a root version that stores a hash is closed, and so is every version up to that index.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_version_open();

    // the versions that remain run from 0 without gaps.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_version_continuous();

    // collapse version runs in an empty chunk,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_collapse_version_context();

    // collapse version needs the root aggregate,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_collapse_version_root_present();

    // each version in the range is declared, and the range ascends,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_collapse_version_present();

    // a version after the range is declared and live. it owns the anonymous body.
    // a range that ends on the last version creates a new version instead.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_collapse_version_owner_present();

    // version before N names the owner, and the next numbered body is that version.
    // a deleted owner keeps only delete; in that body.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_anonymous_versions_owned();

    bool wizard_create_version_context_is_allowed(wizard::context_et) const;

    // a wizard batch runs in the chunk context, otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_batch_context_chunk();

    // create hash runs in an empty chunk,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_create_hash_context();

    // delete hash runs in an empty chunk,
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_delete_hash_context();

    // each listed version is present in the root aggregate declarations.
    // no version means the last version, and that version must exist.
    // otherwise exception::domain::translator_library, exception::subdomain::translator_library::translator
    //
    bool is_wizard_hash_version_present();

    // versions whose stored hash create hash force or delete hash force does not check.
    // empty when the command is absent. no version means the last version
    //
    indices_t hash_force_indexes() const;

    bool wizard_hash_context_is_chunk(wizard::context_et) const;

    bool wizard_create_aggregate_context_is_chunk(wizard::context_et) const;
    bool wizard_create_aggregate_root_aggregate_is_present() const;
    bool wizard_create_aggregate_last_version_is_open() const;
    bool command_file_is_the_aggregate(const path_t& command_file, const name_t& name, const name_ot& directory) const;
    bool new_aggregate_chunk_is_absent(state::vertex_descriptor_t chunk_node, const name_t& name, const name_ot& directory) const;
    path_t new_aggregate_chunk_path(state::vertex_descriptor_t chunk_node, const name_t& name, const name_ot& directory) const;
    bool chunk_path_exists(const path_t&) const;
    bool skips_root_aggregate_checks() const;

    // Snapshot at the moment cpp2cpp_validate_t is created. Checks only read it.
    // status and hash stay on the version container
    struct aggregate_version_declaration_t
    {
        index_t index = 0;
    };

    struct declaration_by_index
    {
        using result_type = const index_t&;
        result_type operator()(const aggregate_version_declaration_t& declaration) const
        {
            return declaration.index;
        }
    };

    struct by_declaration_index
    {};

    using declarations_container_t = boost::multi_index_container
    <
        aggregate_version_declaration_t,
        boost::multi_index::indexed_by
        <
            boost::multi_index::random_access<>,
            boost::multi_index::ordered_unique
            <
                boost::multi_index::tag<by_declaration_index>,
                declaration_by_index
            >
        >
    >;

    class declarations_t : public multi_index_lookup::container_extended_t<declarations_container_t>
    {
    };

    struct aggregate_versions_t
    {
        name_t name;
        state::vertex_descriptor_t vd = {};
        bool is_root = false;
        path_ot chunk_path;
        declarations_t declarations;
        indices_t live_indices;
        indices_t deleted_indices;
        indices_t definition_indices;
        // set on the root. the last declaration that stores a hash
        index_ot last_hashed_index;
    };

    struct aggregate_by_vd
    {
        using result_type = const state::vertex_descriptor_t&;
        result_type operator()(const aggregate_versions_t& aggregate) const
        {
            return aggregate.vd;
        }
    };

    struct aggregate_by_name
    {
        using result_type = const name_t&;
        result_type operator()(const aggregate_versions_t& aggregate) const
        {
            return aggregate.name;
        }
    };

    struct by_aggregate_vd
    {};

    struct by_aggregate_name
    {};

    using aggregate_versions_container_t = boost::multi_index_container
    <
        aggregate_versions_t,
        boost::multi_index::indexed_by
        <
            boost::multi_index::random_access<>,
            boost::multi_index::ordered_unique
            <
                boost::multi_index::tag<by_aggregate_vd>,
                aggregate_by_vd
            >,
            boost::multi_index::ordered_non_unique
            <
                boost::multi_index::tag<by_aggregate_name>,
                aggregate_by_name
            >
        >
    >;

    class aggregate_rows_t : public multi_index_lookup::container_extended_t<aggregate_versions_container_t>
    {
    };

    indices_t declaration_indices(const aggregate_versions_t&) const;
    const aggregate_version_declaration_t* declaration_at(const aggregate_versions_t&, index_t) const;
    version_status_oet declaration_status(const aggregate_versions_t&, index_t) const;
    const aggregate_versions_t* try_aggregate(const name_t&) const;
    bool root_is_present() const;
    index_ot root_last_hashed_index() const;
    const aggregate_versions_t& root_versions() const;
    std::string join_root_chunk_paths(bool filename_only) const;
    std::string join_declaration_statuses(const aggregate_versions_t&) const;

    const state::holder::language_part_t& language_part_;
    validate_memo_t& memo_;
    aggregate_rows_t aggregate_versions_;
    state::vertex_descriptor_ot root_vd_;
};

}// namespace galimatron::translator::transforms::cpp2cpp_validate
