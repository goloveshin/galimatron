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


vertex_descriptor_t builder_t::create_ordering_statement(const create_ordering_statement_t& args)
{
    const auto vd = add_vertex({
        .entity_type = entity_et::ORDERING_STATEMENT,
        .entity = ordering_statement_t{
            .index_in_version = args.index,
            .name = args.name,
            .ordering_parts = args.ordering_parts
        },
        .location = args.location,
        .vd_parent = args.version_definition_node
    });
    state_.note_version_child(vd);
    return vd;
}

}// namespace galimatron::translator::state::builder
