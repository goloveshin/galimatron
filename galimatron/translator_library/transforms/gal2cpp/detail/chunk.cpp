// galimatron
#include "galimatron/translator_library/transforms/gal2cpp/detail/listener.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::gal2cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_g2s);
}


void listener_t::enterChunk(g_parser::ChunkContext * ctx)
{
    push_parent(builder_.create_chunk({
        .root_node = parent(),
        .location = {{
            .path = config_.content_path
        }}
    }));
    assign_user_comments(parent(), ctx);
}

void listener_t::exitChunk(g_parser::ChunkContext * /*ctx*/)
{
    vd_aggregate_.reset();
    pop_parent();
}

}// namespace galimatron::translator::transforms::gal2cpp
