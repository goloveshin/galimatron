// galimatron
#include "galimatron/translator_library/transforms/xml2cpp/detail/xml2cpp_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::xml2cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_x2s);
}


void xml2cpp_t::read_chunk(const pugi::xml_node& node)
{
    const auto vd = builder_.create_chunk({
        .root_node = builder_.root_node(),
        .location = read_location(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

}// namespace galimatron::translator::transforms::xml2cpp
