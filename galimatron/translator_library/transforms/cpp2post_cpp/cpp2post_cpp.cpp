// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/cpp2post_cpp_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_cmp);
}


holder::compiled_part_sptr_t translate(const holder::language_part_sptr_t language_state)
{
    return cpp2post_cpp_t({
        .language_state = language_state
    }).translate();
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
