// galimatron
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"

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


vertex_descriptor_t builder_t::create_label_statement(const create_label_statement_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::LABEL_STATEMENT,
        .entity = label_statement_t{
            .index_in_version = args.index,
            .name = args.name
        },
        .location = args.location,
        .vd_parent = args.version_definition_node
    });
    state_.note_version_child(vd);
    return vd;
}

vertex_descriptor_t builder_t::create_label_dynamic(const create_label_dynamic_t& args)
{
    return add_vertex({
        .entity_type = entity_et::LABEL_DYNAMIC,
        .entity = label_dynamic_t{
            .value = args.value
        },
        .location = {},
        .vd_parent = args.label_node
    });
}

vertex_descriptor_t builder_t::create_label_static(const create_label_static_t& args)
{
    const auto node = add_vertex({
        .entity_type = entity_et::LABEL_STATIC,
        .entity = label_static_t{
            .values = args.values,
            .tag_name_from = args.tag_name_from
        },
        .location = {},
        .vd_parent = args.label_node
    });

    return node;
}

}// namespace galimatron::translator::state::builder
