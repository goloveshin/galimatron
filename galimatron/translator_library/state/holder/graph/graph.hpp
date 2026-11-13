#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/graph/container.hpp"
#include "galimatron/translator_library/state/holder/graph/vertex.hpp"


namespace galimatron::translator::state::holder::vertex_graph
{

struct containerS
{
};

struct container_tag_t :
    virtual public boost::graph_detail::random_access_container_tag,
    virtual public boost::graph_detail::back_insertion_sequence_tag
{};

}// namespace galimatron::translator::state::holder::vertex_graph


namespace boost
{

namespace ls = galimatron::translator::state::holder;
template <typename StoredVertexType>
using language_container_t = ls::vertex_container::container_t<StoredVertexType>;
using language_container_tag_t = ls::vertex_graph::container_tag_t;
using language_containerS = ls::vertex_graph::containerS;

template <typename StoredVertexType>
inline language_container_tag_t container_category(const language_container_t<StoredVertexType>)
{
    return {};
}

template <typename StoredVertexType>
struct container_gen<language_containerS, StoredVertexType>
{
    using type = language_container_t<StoredVertexType>;
};

namespace detail
{

template <>
struct is_random_access<language_containerS>
{
    static constexpr bool value = true;
    using type = boost::mpl::true_;
};

}// namespace detail

}// namespace boost


namespace galimatron::translator::state::holder::vertex_graph
{

using graph_t = boost::adjacency_list
<
    boost::listS,// OutEdgeList
    containerS,// VertexList
    boost::bidirectionalS,// Directed
    vertex_t // VertexProperty
>;

static_assert(std::same_as<vertex_descriptor_t, boost::graph_traits<graph_t>::vertex_descriptor>, "must be same");

}// namespace galimatron::translator::state::holder::vertex_graph
