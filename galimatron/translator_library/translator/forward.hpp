#pragma once

// galimatron
#include "galimatron/translator_library/galimatron_config_declaration.hpp"


namespace galimatron::translator::translator
{

namespace detail
{

using sequential_number_t = index_t;

// named transform
enum class named_et
{
    GAL2CPP,
    CPP2GAL
};
using named_oet = std::optional<named_et>;

// where the transform was declared
enum class origin_et
{
    BUILD_IN,
    USER
};

enum class status_et
{
    SUCCESS,
    FAILED,
    SKIPPED
};

struct translate_result_t
{
    // number the transforms
    sequential_number_t sequential_number;

    // for the log
    name_t name;

    origin_et origin;

    // named transform
    named_oet named;

    status_et status;

    // result
    file_content_with_path_ot produced_xml;
    file_contents_with_paths_ot produced_files;

    // generating transform
    const galimatron_config::transform_t& config_transform;
};
using notify_transform_result_ft = std::function<void (const translate_result_t&)>;

// the taskflow graph text, after the run
using notify_taskflow_graph_ft = std::function<void (const content_t&)>;

// private memory added while the translator state is alive, in bytes
using notify_state_memory_ft = std::function<void (const std::size_t)>;

}// namespace detail

}// namespace galimatron::translator::translator
