#include "test_common.hpp"

using namespace majorminer;


void majorminer::containsEdges(const graph_t& graph, std::initializer_list<edge_t> edges)
{
  tbb::parallel_for_each(edges.begin(), edges.end(),
    [&graph](const edge_t& edge) {
      EXPECT_TRUE(graph.find(edge) != graph.end()
        || graph.find(std::make_pair(edge.second, edge.first)) != graph.end());
  });
}

void majorminer::printGraph(const graph_t& graph)
{
  for (const auto& p : graph)
  {
    std::cout << "(" << p.first << ", " << p.second << ")" << std::endl;
  }
}


void majorminer::addEdges(graph_t& graph, std::initializer_list<edge_t> edges)
{
  for (const auto& edge : edges)
  {
    graph.insert(edge);
  }
}