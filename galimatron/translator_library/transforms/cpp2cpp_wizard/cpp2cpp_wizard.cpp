// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/cpp2cpp_wizard.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2s);
}


holder::language_part_sptr_t translate(const holder::language_part_sptr_t language_state)
{
    if (!language_state || language_state->wizard_statements().empty())
    {
        return language_state;
    }

    cpp2cpp_wizard_t(*language_state).translate();
    language_state->drop_detached_vertices();
    return language_state;
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
