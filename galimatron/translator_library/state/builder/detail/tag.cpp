// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::state::builder
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_bld);
}


vertex_descriptor_t builder_t::create_tag_statement(const create_tag_statement_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::TAG_STATEMENT,
        .entity = tag_statement_t{
            .index_in_version = args.index_in_version,
            .name = args.name,
            .labels = args.labels,
            .mode = args.mode,
            .string_values_at_zero_level = {}
        },
        .location = args.location,
        .vd_parent = args.version_definition_node
    });
    state_.note_version_child(vd);
    return vd;
}

vertex_descriptor_t builder_t::create_tag_values(const create_tag_values_t& args)
{
    return add_vertex({
        .entity_type = entity_et::TAG_VALUES,
        .entity = tag_values_t{},
        .location = {},
        .vd_parent = args.parent_node
    });
}

vertex_descriptor_t builder_t::create_tag_value(const create_tag_value_t& args)
{
    return add_vertex({
        .entity_type = entity_et::TAG_VALUE,
        .entity = tag_value_t{
            .index = args.index
        },
        .location = {},
        .vd_parent = args.parent_node
    });
}

vertex_descriptor_t builder_t::create_tag_record(const create_tag_record_t& args)
{
    return add_vertex({
        .entity_type = entity_et::TAG_RECORD,
        .entity = tag_record_t{},
        .location = {},
        .vd_parent = args.parent_node
    });
}

vertex_descriptor_t builder_t::create_tag_keyvalue(const create_tag_keyvalue_t& args)
{
    return add_vertex({
        .entity_type = entity_et::TAG_KEYVALUE,
        .entity = tag_keyvalue_t{
            .key = args.key,
            .key_delimiter = args.key_delimiter
        },
        .location = {},
        .vd_parent = args.parent_node
    });
}

vertex_descriptor_t builder_t::create_tag_array(const create_tag_array_t& args)
{
    return add_vertex({
        .entity_type = entity_et::TAG_ARRAY,
        .entity = tag_array_t{},
        .location = {},
        .vd_parent = args.parent_node
    });
}

vertex_descriptor_t builder_t::create_tag_string(const create_tag_string_t& args)
{
    return add_vertex({
        .entity_type = entity_et::TAG_STRING,
        .entity = tag_string_t{
            .value = args.value,
            .delimiter = args.delimiter,
            .is_template = args.is_template
        },
        .location = {},
        .vd_parent = args.parent_node
    });
}

}// namespace galimatron::translator::state::builder
