#include "utils.hpp"

using namespace majorminer;



void majorminer::convertToAdjacencyList(adjacency_list_t& adj, const graph_t& graph)
{
  tbb::parallel_for_each(graph.begin(), graph.end(),
    [&adj](const edge_t& edge){
      adj.insert(std::make_pair(edge.first, edge.second));
      adj.insert(std::make_pair(edge.second, edge.first));
  });
}

