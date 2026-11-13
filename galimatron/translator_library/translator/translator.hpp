#pragma once

// galimatron
#include "galimatron/translator_library/translator/forward.hpp"


namespace galimatron::translator::translator
{

namespace detail
{

// settings that did not fall into other groups
//
struct galimatron_t
{
    // galimatron_summary.xml and the taskflow graph go here
    path_t intermediate_summary_dir;

    // omit the transform time from the info log. absent means false
    bool committed_report = false;
};

// builtin steps of the testing pipeline. a flag adds that step
//
struct testing_t
{
    bool gal2cpp = true;
    bool cpp2cpp_validate = true;
    bool cpp2cpp_wizard = true;
    bool cpp2xml = true;
    bool xml2cpp = true;
    bool cpp2gal = true;
    bool cpp2post_cpp = true;
    bool post_cpp2post_xml = true;

    // forget stored tag and ordering text before cpp2gal, so the printers use the default layout
    bool drop_tag_user_formatting = false;
};

}// namespace detail

// the part of galimatron_config that translate reads, plus the incoming GaL state
//
struct config_t
{
public:// input

    // general settings
    detail::galimatron_t galimatron;

    // Saxon processor settings
    galimatron_config::detail::saxon_processor_t saxon_processor;

    // transforms
    galimatron_config::detail::transforms::transforms_t transforms;

    // incoming GaL state (used once)
    deferred_file_contents_with_paths_t gal_state;

    // number from gal.version
    index_t gal_version;

    // splits a location path into parts
    gf::split_path_by_parts_ft split_path_by_parts;

    // testing pipeline. absent: production
    std::optional<detail::testing_t> testing;

public:// output

    // report the result of one transform
    detail::notify_transform_result_ft notify_transform_result;

    // the taskflow graph after the run. the caller stores taskflow.dot
    detail::notify_taskflow_graph_ft notify_taskflow_graph;

    // private memory added while the translator state is alive, in bytes
    detail::notify_state_memory_ft notify_state_memory;
};

// the taskflow graph, written into intermediate-summary-dir after the run
inline constexpr std::string_view taskflow_graph_file_name = "taskflow.dot";

// Over the Galimatron state (.gal files), we run the transforms
// specified in .gal files (wizard) and in the config (user xslt).
//
// NOTE: Galimatron is a file utility - files in, files out!
//
void translate(const config_t&);

}// namespace galimatron::translator::translator
