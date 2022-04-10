#include "common/utils.hpp"

#include <common/embedding_base.hpp>

using namespace majorminer;



void majorminer::convertToAdjacencyList(adjacency_list_t& adj, const graph_t& graph)
{
  tbb::parallel_for_each(graph.begin(), graph.end(),
    [&adj](const edge_t& edge){
      adj.insert(std::make_pair(edge.first, edge.second));
      adj.insert(std::make_pair(edge.second, edge.first));
  });
}


void majorminer::insertMappedTargetNodes(const EmbeddingBase& base, nodeset_t& nodes, fuint32_t sourceNode)
{
  const auto& mapping = base.getMapping();
  auto equalRange = mapping.equal_range(sourceNode);
  for (auto it = equalRange.first; it != equalRange.second; ++it) nodes.insert(it->second);
}

nodeset_t majorminer::getVertices(const graph_t& graph)
{
  nodeset_t vertices{};
  for (const auto& edge : graph)
  {
    vertices.insert(edge.first);
    vertices.insert(edge.second);
  }
  return vertices;
}