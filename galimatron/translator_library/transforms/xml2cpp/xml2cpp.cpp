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


holder::language_part_sptr_t translate(const translate_t& args)
{
    return xml2cpp_t(args).translate();
}

}// namespace galimatron::translator::transforms::xml2cpp
