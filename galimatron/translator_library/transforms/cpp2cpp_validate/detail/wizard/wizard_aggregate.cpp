// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/detail/support.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/state/wizard/tags.hpp"
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/exception_augment.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"

// Boost.LEAF
#include <boost/leaf/handle_errors.hpp>

// global
#include "global/common_library/utils.hpp"

// leaf_spy
#include "captured_leaf_exception.hpp"

// Taskflow
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <taskflow/taskflow.hpp>

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2cpp_validate
{

using namespace galimatron::translator::state;
using galimatron::translator::galimatron_config::report_path;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

// create aggregate context is chunk
//
bool cpp2cpp_validate_t::wizard_create_aggregate_context_is_chunk(const wizard::context_et context) const
{
    return context == wizard::context_et::CHUNK;
}

// the root aggregate already exists
//
bool cpp2cpp_validate_t::wizard_create_aggregate_root_aggregate_is_present() const
{
    return !wizard_create_root_aggregate_root_aggregate_is_absent();
}

// no hash covers the last root version. no root means the later check reports that
//
bool cpp2cpp_validate_t::wizard_create_aggregate_last_version_is_open() const
{
    if (!root_is_present())
    {
        return true;
    }

    const auto closed = root_last_hashed_index();
    if (!closed)
    {
        return true;
    }

    const auto& declarations = root_versions().declarations;
    if (declarations.empty())
    {
        return true;
    }

    return declarations.back().index > *closed;
}

path_t cpp2cpp_validate_t::new_aggregate_chunk_path(
    const state::vertex_descriptor_t chunk_node,
    const name_t& name,
    const name_ot& directory
) const
{
    const auto& chunk = language_part_.graph()[chunk_node];
    auto path = chunk.location->path->parent_path();
    if (directory)
    {
        path /= *directory;
    }
    return path / (name + ".gal");
}

// the command file is the file the new aggregate would be written to
//
bool cpp2cpp_validate_t::command_file_is_the_aggregate(
    const path_t& command_file,
    const name_t& name,
    const name_ot& directory
) const
{
    const auto& command = wizard_statement(language_part_);
    const auto target = new_aggregate_chunk_path(command.chunk_node, name, directory);
    return boost::iequals(command_file.generic_string(), target.generic_string());
}

// a chunk with this path is already loaded
//
bool cpp2cpp_validate_t::chunk_path_exists(const path_t& path) const
{
    const auto& graph = language_part_.graph();
    const auto edges = boost::out_edges(language_part_.root_node(), graph);
    for (auto edge = edges.first; edge != edges.second; ++edge)
    {
        const auto& vertex = graph[boost::target(*edge, graph)];
        if (vertex.entity_type != state::entity_et::CHUNK)
        {
            continue;
        }
        if (vertex.location && vertex.location->path && *vertex.location->path == path)
        {
            return true;
        }
    }
    return false;
}

// name.gal is not already loaded
//
bool cpp2cpp_validate_t::new_aggregate_chunk_is_absent(
    const state::vertex_descriptor_t chunk_node,
    const name_t& name,
    const name_ot& directory
) const
{
    return !chunk_path_exists(new_aggregate_chunk_path(chunk_node, name, directory));
}

}// namespace galimatron::translator::transforms::cpp2cpp_validate
