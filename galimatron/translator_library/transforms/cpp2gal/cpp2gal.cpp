// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2gal
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2g);
}


file_contents_with_paths_t translate(const translate_t& args)
{
    return cpp2gal_t(args).translate();
}

}// namespace galimatron::translator::transforms::cpp2gal
