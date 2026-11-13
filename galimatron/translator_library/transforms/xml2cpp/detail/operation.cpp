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


void xml2cpp_t::read_operation_statement(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto name = attribute_if(node, tag_c::name);

    const auto vd = builder_.create_operation_statement({
        .version_definition_node = parent,
        .index = attribute_index(node),
        .name = name.value_or(""),
        .is_version = !name,
        .location = read_location(node)
    });
    restore_layout(node, vd);
}

}// namespace galimatron::translator::transforms::xml2cpp
