// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


translator_t::translator_t(const config_t& config)
    : args_(config)
    , language_transforms_(args_.transforms.language_transforms)
    , post_language_transforms_(args_.transforms.post_language_transforms)
{
    // dump() prints this instead of the Taskflow object's address
    taskflow_.name("transformations");
    fLogmeD("translator created");
}

translator_t::~translator_t()
{
    fLogmeD("translator destroyed");
}

tf::Executor& translator_t::executor()
{
    static tf::Executor executor{ std::thread::hardware_concurrency() };
    return executor;
}

}// namespace galimatron::translator::translator
