// galimatron
#include "galimatron/translator_library/state/holder/ordering_rank.hpp"

// global
#include "global/common_library/utils.hpp"

// Boost.Graph
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/transitive_closure.hpp>


namespace galimatron::translator::state
{

namespace
{

struct back_edge_t
{
    bool found = false;
    std::size_t source = 0;
    std::size_t target = 0;
};

// records the first back edge of a depth-first search
//
template <typename Graph>
struct back_edge_visitor : boost::default_dfs_visitor
{
    back_edge_t& edge;

    back_edge_visitor(back_edge_t& found)
        : edge(found)
    {}

    template <typename Edge>
    void back_edge(const Edge& edge_found, const Graph& graph) const
    {
        if (edge.found)
        {
            return;
        }
        edge.found = true;
        edge.source = boost::source(edge_found, graph);
        edge.target = boost::target(edge_found, graph);
    }
};

// the parts as one directed graph. a repeated name is the same vertex
//
struct ordering_graph_t
{
    using graph_t = boost::adjacency_list<boost::setS, boost::vecS, boost::directedS>;

    graph_t graph;
    std::map<name_t, graph_t::vertex_descriptor> known;
    std::vector<name_t> name_of;

    // one part is a chain of edges
    //
    void add(const ordering_statement_t::ordering_part_t& part)
    {
        for (const auto& name : part)
        {
            auto [where, inserted] = known.try_emplace(name, name_of.size());
            if (inserted)
            {
                name_of.push_back(name);
                boost::add_vertex(graph);
            }
        }
        const auto rest = part | ranges::views::drop(1);
        for (const auto& [before, after] : ranges::views::zip(part, rest))
        {
            boost::add_edge(known.at(before), known.at(after), graph);
        }
    }
};

// the rank of a vertex is the longest chain of predecessors
//
template <typename Graph>
std::vector<index_t> longest_ranks(
    const Graph& graph,
    const std::vector<typename Graph::vertex_descriptor>& order
)
{
    std::vector<index_t> rank(order.size(), 0);
    for (const auto vertex : order | ranges::views::reverse)
    {
        for (const auto neighbour : subrange(boost::adjacent_vertices(vertex, graph)))
        {
            rank[neighbour] = std::max(rank[neighbour], rank[vertex] + 1);
        }
    }
    return rank;
}

// a smaller rank without a path is not a strict weak order.
// the pair is the lexicographically smallest
//
template <typename Graph>
std::optional<ordering_rank_problem_t> rank_mismatch(
    const Graph& closed,
    const std::vector<name_t>& name_of,
    const std::vector<index_t>& rank
)
{
    const auto indexes = ranges::views::iota(std::size_t{ 0 }, name_of.size());
    auto pairs = ranges::views::cartesian_product(indexes, indexes);
    auto failing =
        pairs
        | ranges::views::filter([&](const auto& ends)
        {
            const auto [left, right] = ends;
            return rank[left] < rank[right] && !boost::edge(left, right, closed).second;
        })
        | ranges::views::transform([&](const auto& ends)
        {
            const auto [left, right] = ends;
            return std::pair{ name_of[left], name_of[right] };
        })
        ;

    const auto where = ranges::min_element(failing);
    if (where == ranges::end(failing))
    {
        return std::nullopt;
    }
    const auto& pair = *where;
    return ordering_rank_problem_t{
        .cycle = false,
        .first = pair.first,
        .second = pair.second,
    };
}

// the back edge names the cycle
//
template <typename Graph>
ordering_rank_problem_t cycle_problem(const Graph& graph, const std::vector<name_t>& name_of)
{
    back_edge_t edge;
    std::vector<boost::default_color_type> color(boost::num_vertices(graph));
    back_edge_visitor<Graph> visitor(edge);
    boost::depth_first_search(
        graph,
        visitor,
        boost::make_iterator_property_map(color.begin(), boost::get(boost::vertex_index, graph))
    );
    return ordering_rank_problem_t{
        .cycle = true,
        .first = edge.found ? name_of[edge.source] : name_t{},
        .second = edge.found ? name_of[edge.target] : name_t{},
    };
}

// the rank of each named aggregate
//
std::map<name_t, index_t> ranks_by_name(
    const std::vector<name_t>& name_of,
    const std::vector<index_t>& rank
)
{
    auto named = ranges::views::enumerate(rank);
    return
        named
        | ranges::views::transform([&](const auto& item)
        {
            const auto& [vertex, value] = item;
            return std::pair{ name_of[vertex], value };
        })
        | ranges::to<std::map<name_t, index_t>>()
        ;
}

}// namespace

ordering_ranks_t ranks_of_ordering(const ordering_statement_t::ordering_parts_t& parts)
{
    ordering_graph_t ordering;
    for (const auto& part : parts)
    {
        ordering.add(part);
    }

    ordering_ranks_t result;
    if (ordering.name_of.empty())
    {
        return result;
    }

    try
    {
        std::vector<ordering_graph_t::graph_t::vertex_descriptor> order;
        boost::topological_sort(ordering.graph, std::back_inserter(order));

        const auto rank = longest_ranks(ordering.graph, order);
        ordering_graph_t::graph_t closed;
        boost::transitive_closure(ordering.graph, closed);
        if (const auto problem = rank_mismatch(closed, ordering.name_of, rank))
        {
            result.problem = problem;
            return result;
        }
        result.rank = ranks_by_name(ordering.name_of, rank);
    }
    catch (const boost::not_a_dag&)
    {
        result.problem = cycle_problem(ordering.graph, ordering.name_of);
    }
    return result;
}

}// namespace galimatron::translator::state
