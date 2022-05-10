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


fuint32_pair_t majorminer::calculateOverlappingStats(const EmbeddingBase& base)
{
  fuint32_t distinct = 0;
  fuint32_t total = 0;
  nodeset_t targetVertices = getVertices(*base.getTargetGraph());
  const auto& reverse = base.getReverseMapping();

  for (auto target : targetVertices)
  {
    fuint32_t nbMapped = reverse.count(target);
    if (nbMapped >= 2)
    {
      distinct++;
      total += nbMapped;
    }
  }
  return std::make_pair(distinct, total);
}

embedding_mapping_t majorminer::replaceMapping(const embedding_mapping_t& mapping,
    const nodeset_t& targets, vertex_t source)
{
  embedding_mapping_t adjusted{};
  adjusted.insert(mapping.begin(), mapping.end());
  adjusted.unsafe_erase(source);
  for (auto target : targets)
  {
    adjusted.insert(std::make_pair(source, target));
  }
  return adjusted;
}

fuint32_t majorminer::calculateFitness(const EmbeddingBase& state, const nodeset_t& superVertex)
{
  const auto& reverse = state.getReverseMapping();
  fuint32_t fitness = 0;
  for (vertex_t target : superVertex)
  {
    fitness += reverse.count(target);
  }
  return fitness;
}

bool majorminer::containsEdge(const graph_t& graph, edge_t edge)
{
  return graph.contains(orderedPair(edge));
}

