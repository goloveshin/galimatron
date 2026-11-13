#pragma once

// galimatron
#include "galimatron/translator_library/state/forward.hpp"
#include "galimatron/translator_library/state/tag_mode.hpp"


namespace galimatron::translator::state
{

//------------------------------------------------------------------------------
// Root
//------------------------------------------------------------------------------

struct root_t
{
};

//------------------------------------------------------------------------------
// Chunk
//------------------------------------------------------------------------------

struct chunk_t
{
};

//------------------------------------------------------------------------------
// Root Aggregate
//------------------------------------------------------------------------------

struct root_aggregate_t
{
    name_t name;
    // the wizard created this root aggregate
    bool created_by_wizard = false;
};

struct root_aggregate_version_declarations_t
{
};

struct root_aggregate_version_declaration_t
{
    index_t index;
    version_status_oet status;
    string_ot comment;
    string_ot hash;
};

struct root_aggregate_version_definitions_t
{
};

struct root_aggregate_version_definition_t
{
    // empty: anonymous body, owned by owner
    index_ot index;
    index_ot owner;
    // the wizard added this version to a root aggregate that already existed
    bool created_by_wizard = false;
    // once drops earlier root versions. local is refused on the root
    tag_mode_et mode = tag_mode_et::INHERIT;
};

//------------------------------------------------------------------------------
// Aggregate
//------------------------------------------------------------------------------

struct aggregate_t
{
    name_t name;
    index_t index;
    // the wizard created this aggregate
    bool created_by_wizard = false;
};

struct aggregate_version_declarations_t
{
};

struct aggregate_version_declaration_t
{
    index_t index;
    version_status_oet status;
    string_ot comment;
    string_ot hash;
};

struct aggregate_version_definitions_t
{
};

struct aggregate_version_definition_t
{
    // empty: anonymous body, owned by owner
    index_ot index;
    index_ot owner;
    // the wizard added this version to an aggregate that already existed
    bool created_by_wizard = false;
    // once drops earlier versions of this aggregate and still receives the root.
    // local drops both. an absent mode inherits
    tag_mode_et mode = tag_mode_et::INHERIT;
};

//------------------------------------------------------------------------------
// Tag
//------------------------------------------------------------------------------

struct tag_statement_t
{
    index_t index_in_version;
    name_ot name;
    std::deque<name_t> labels;
    tag_mode_et mode = tag_mode_et::INHERIT;
    std::map<index_t, vertex_descriptor_t> string_values_at_zero_level;
    // the wizard copied this tag into the new version
    bool created_by_wizard = false;
};
using tag_statement_rt = std::reference_wrapper<tag_statement_t>;
using tag_statement_ort = std::optional<tag_statement_rt>;

struct tag_values_t
{
};

struct tag_value_t
{
    index_ot index;
};

struct tag_record_t
{
};

// the quotes a source string was written with. the text itself stays unquoted
//
struct string_delimiter_t
{
    bool raw = false;
    char quote = '"';
    std::string dashes;
    bool operator==(const string_delimiter_t&) const = default;
};

struct tag_keyvalue_t
{
    string_ot key;
    // index into language_part_t string delimiters. 0 is a plain double quote
    index_t key_delimiter = 0;
};

struct tag_array_t
{
};

struct tag_string_t
{
    std::string value;
    // index into language_part_t string delimiters. 0 is a plain double quote
    index_t delimiter = 0;
    // the source string was marked _te. apply() renders it and clears this
    bool is_template = false;
};

//------------------------------------------------------------------------------
// Label
//------------------------------------------------------------------------------

struct label_statement_t
{
    index_t index_in_version;
    name_t name;
};

struct label_dynamic_t
{
    std::string value;
};

struct label_static_t
{
    strings_t values;
    string_ot tag_name_from;
};

//------------------------------------------------------------------------------
// Ordering
//------------------------------------------------------------------------------

struct ordering_statement_t
{
    index_t index_in_version;
    name_t name;

    using ordering_part_t = std::deque<name_t>;
    using ordering_parts_t = std::deque<ordering_part_t>;
    ordering_parts_t ordering_parts;
};

//------------------------------------------------------------------------------
// Operation
//------------------------------------------------------------------------------

// historical deletion.
// delete <name>; — a tag: from this point the tag is absent from the compiled state.
// delete; — an aggregate version: the rest of the version contents are removed, the version is deleted.
// physical deletion is different.
// included in xml: this is a GaL element, unparse prints it
//
struct operation_statement_t
{
    index_t index_in_version;
    name_t name;
    // true: delete; in an aggregate version definition
    bool is_version = false;
};

//------------------------------------------------------------------------------
// Wizard
//------------------------------------------------------------------------------

// a wizard command. after it runs, its text stays on the chunk as // comments.
// the statement itself is absent from xml
//
struct wizard_statement_t
{
    wizard::command_et type;
    wizard::context_et context;
    vertex_descriptor_t chunk_node = vertex_descriptor_not_present;
    path_t chunk_uri;
    wizard::command_data_t data;
    // filled when type is BATCH. empty for one command
    std::vector<wizard::batch_command_t> commands;
};

//------------------------------------------------------------------------------

using entity_t = std::variant
<
    root_t,
    chunk_t,

    root_aggregate_t,
    root_aggregate_version_declarations_t,
    root_aggregate_version_declaration_t,
    root_aggregate_version_definitions_t,
    root_aggregate_version_definition_t,

    aggregate_t,
    aggregate_version_declarations_t,
    aggregate_version_declaration_t,
    aggregate_version_definitions_t,
    aggregate_version_definition_t,

    tag_statement_t,
    tag_values_t,
    tag_value_t,
    tag_record_t,
    tag_keyvalue_t,
    tag_array_t,
    tag_string_t,

    label_statement_t,
    label_dynamic_t,
    label_static_t,

    ordering_statement_t,

    operation_statement_t,

    wizard_statement_t
>;

enum class entity_et
{
    ROOT,
    CHUNK,

    ROOT_AGGREGATE,
    ROOT_AGGREGATE_VERSION_DECLARATIONS,
    ROOT_AGGREGATE_VERSION_DECLARATION,
    ROOT_AGGREGATE_VERSION_DEFINITIONS,
    ROOT_AGGREGATE_VERSION_DEFINITION,

    AGGREGATE,
    AGGREGATE_VERSION_DECLARATIONS,
    AGGREGATE_VERSION_DECLARATION,
    AGGREGATE_VERSION_DEFINITIONS,
    AGGREGATE_VERSION_DEFINITION,

    TAG_STATEMENT,
    TAG_LABELS,
    TAG_VALUES,
    TAG_VALUE,
    TAG_RECORD,
    TAG_KEYVALUE,
    TAG_ARRAY,
    TAG_STRING,

    LABEL_STATEMENT,
    LABEL_DYNAMIC,
    LABEL_STATIC,

    ORDERING_STATEMENT,

    OPERATION_STATEMENT,

    WIZARD_STATEMENT
};

std::string to_string(const entity_et&);

}// namespace galimatron::translator::state
