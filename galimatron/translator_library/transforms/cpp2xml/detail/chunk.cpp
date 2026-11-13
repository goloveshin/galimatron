// galimatron
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2xml
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2x);
}


namespace tag = galimatron::translator::xml_tags::file;

pugi::xml_node cpp2xml_t::create_chunk(const chunk_t& /* args */)
{
    auto node = parent().append_child(tag::chunk);

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
