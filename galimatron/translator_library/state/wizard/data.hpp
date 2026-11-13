#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"
#include "galimatron/translator_library/state/tag_mode.hpp"
#include "galimatron/translator_library/state/wizard/command.hpp"


namespace galimatron::translator::state::wizard
{

struct create_aggregate_data_t
{
    strings_t names;
    // create aggregate in <directory> writes name.gal there. empty means next to the command file
    name_ot directory;
    // delete aggregate physically removes the named chunks
    bool physically = false;
    // create aggregate once or local marks the last version. an absent mode inherits
    tag_mode_et mode = tag_mode_et::INHERIT;
};

// wizard copy aggregate <source> to <name> [, <name>]* [in <directory>]
//
struct copy_aggregate_data_t
{
    name_t source;
    strings_t names;
    // in <directory> writes name.gal there. empty means next to the command file
    name_ot directory;
};

// wizard continue aggregate <source> to <name> [, <name>]* [in <directory>]
//
struct continue_aggregate_data_t
{
    name_t source;
    strings_t names;
    // in <directory> writes name.gal there. empty means next to the command file
    name_ot directory;
};

struct create_version_data_t
{
    // delete version <list>. empty means the last version. create version leaves this empty
    indices_t version_indexes;
    // create version once or local. an absent mode follows the previous version of each aggregate
    tag_mode_et mode = tag_mode_et::INHERIT;
};

struct hash_data_t
{
    indices_t version_indexes;
    // create hash force stores the hash as the important tags are now.
    // delete hash force removes it. the validation before the wizard skips that version
    bool force = false;
};

// wizard collapse version <first>-<last>.
// a following version owns version before. the last version parks into a new version
//
struct collapse_version_data_t
{
    index_t first = 0;
    index_t last = 0;
};

// one tag value, or one field of a record. a string, a record, or an array
//
struct json_t
{
    name_ot key;
    index_t key_delimiter = 0;
    name_ot string;
    index_t delimiter = 0;
    bool is_template = false;
    bool is_array = false;
    std::vector<json_t> elements;
};

// aggregate is set in a chunk. a version definition leaves it empty
//
struct create_tag_data_t
{
    name_ot aggregate;
    name_ot name;
    strings_t labels;
    tag_mode_et mode = tag_mode_et::INHERIT;
    std::vector<json_t> values;
};

struct label_data_t
{
    name_ot aggregate;
    name_t name;
    bool is_dynamic = false;
    strings_t static_values;
    name_ot tag_name_from;
    name_ot dynamic_value;
};

struct ordering_data_t
{
    name_ot aggregate;
    name_t name;
    std::vector<strings_t> parts;
};

struct delete_tag_data_t
{
    name_ot aggregate;
    name_t name;
};

using command_data_t = std::variant<
    create_aggregate_data_t,
    copy_aggregate_data_t,
    continue_aggregate_data_t,
    create_version_data_t,
    hash_data_t,
    collapse_version_data_t,
    create_tag_data_t,
    label_data_t,
    ordering_data_t,
    delete_tag_data_t
>;

// one command inside a wizard batch. command_data_t stays non-recursive: the batch holds these beside it
//
struct batch_command_t
{
    command_et type;
    command_data_t data;
};

}// namespace galimatron::translator::state::wizard
