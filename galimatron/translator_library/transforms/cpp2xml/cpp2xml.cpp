// galimatron
#include "galimatron/translator_library/transforms/cpp2xml/cpp2xml.hpp"
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


pugi_document_with_meanful_node_t translate(const translate_t& args)
{
    return cpp2xml_t(args).translate();
}

}// namespace galimatron::translator::transforms::cpp2xml
