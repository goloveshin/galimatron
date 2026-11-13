// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/detail/taskflow_dump.hpp"
#include "galimatron/translator_library/parser/gal_version.hpp"

// global
#include "global/common_library/log_subsystems.hpp"
#include "global/common_library/private_memory.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


void translator_t::translate()
{
    translate_internal();
}

void translator_t::translate_internal()
{
    const auto memory_at_begin = global::private_memory_bytes();

    // the destructor runs after the other locals, before translator_t members are destroyed
    auto report_state_memory = scope_guard_t(
        []{},
        [&]
        {
            if (!memory_at_begin || !args_.notify_state_memory)
            {
                return;
            }

            const auto memory_at_end = global::private_memory_bytes();
            if (!memory_at_end)
            {
                return;
            }

            const auto bytes = *memory_at_end >= *memory_at_begin
                ? *memory_at_end - *memory_at_begin
                : std::size_t{0};
            args_.notify_state_memory(bytes);
        }
    );

    // an unknown version is thrown on this thread, not on a worker
    parser::gal_version::require_supported(args_.gal_version);
    fLogmeI("GaL version: {}", args_.gal_version);

    prepare_transforms();

    // precede chain: stages run strictly in order.
    // The graph runs on all cores.
    taskflow_.clear();

    tf::Task previous;
    index_t index = 0;
    for (const transform_t& tf : transforms_)
    {
        auto task = taskflow_.emplace([this, index, &tf](tf::Subflow& subflow)
        {
            execute_transform(index, tf, subflow);
        }).name(tf.config_transform.name);

        if (!previous.empty())
        {
            previous.precede(task);
        }
        previous = task;
        ++index;
    }

    // after the run, including when the run throws: retained subflows are in the graph
    auto notify_taskflow_graph = scope_guard_t(
        []{},
        [&]
        {
            args_.notify_taskflow_graph(readable_taskflow_dump(taskflow_.dump()));
        }
    );

    fLogmeI("begin of transformations:");
    {
        executor().run(taskflow_).get();// wait() does not propagate the task exception
    }
    fLogmeI("end of transformations");
}

}// namespace galimatron::translator::translator
