#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/graph/graph.hpp"


namespace galimatron::translator::state::holder::vertex_payload
{

// entity text of one vertex and its children. formatting and locations are left out
//
std::string vertex(const vertex_graph::graph_t& graph, vertex_descriptor_t vd);

// entity text of the children of a vertex. an empty parent is an empty string
//
std::string children(const vertex_graph::graph_t& graph, vertex_descriptor_ot vd);

}// namespace galimatron::translator::state::holder::vertex_payload
