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


namespace tag = gal_and_xml_tags::operation;

pugi::xml_node cpp2xml_t::create_operation_statement(const operation_statement_t& args)
{
    auto node = parent().append_child(tag::delete_tag);
    {
        node.append_attribute(tag_c::index) = args.index_in_version;
        if (!args.is_version)
        {
            node.append_attribute(tag_c::name) = args.name;
        }
    }

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
