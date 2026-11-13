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


vertex_descriptor_t builder_t::create_root(const create_root_t& args)
{
    return add_vertex({
        .entity_type = entity_et::ROOT,
        .entity = root_t{},
        .location = {},
        .vd_parent = vertex_descriptor_not_present
    });
}

}// namespace galimatron::translator::state::builder
