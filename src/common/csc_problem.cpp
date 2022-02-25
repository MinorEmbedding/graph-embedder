#include "common/csc_problem.hpp"

#include <common/cut_vertex.hpp>
#include <common/embedding_base.hpp>

using namespace majorminer;

adjacency_list_t majorminer::extractSubgraph(const EmbeddingBase& base, fuint32_t sourceNode)
{
  adjacency_list_t subgraph{};
  const auto& targetGraph = *base.getTargetGraph();
  base.iterateSourceMappingPair(sourceNode,
    [&subgraph, &targetGraph](fuint32_t targetNodeA, fuint32_t targetNodeB){
      edge_t uv{targetNodeA, targetNodeB};
      edge_t vu{targetNodeB, targetNodeA};
      if (targetGraph.contains(uv) || targetGraph.contains(vu))
      {
        subgraph.insert(uv);
        subgraph.insert(vu);
      }
  });
  return subgraph;
}


bool majorminer::isNodeCrucial(const EmbeddingBase& base, fuint32_t sourceNode, fuint32_t targetNode)
{
  // 1. Check whether targetNode is crucial due to it being a cut vertex
  if (isCutVertex(base, sourceNode, targetNode)) return true;

  // 1. Check whether targetNode is crucial connections to other super vertices
  nodeset_t sourceConnections{};
  base.iterateSourceGraphAdjacent(sourceNode, [&](fuint32_t adjSourceNode){
    sourceConnections.insert(adjSourceNode);
  });

  nodeset_t connections{};
  base.iterateTargetAdjacentReverseMapping(targetNode, [&](fuint32_t adjSourceNode){
    if (sourceConnections.contains(adjSourceNode)) connections.insert(adjSourceNode);
  });

  base.iterateSourceMappingAdjacentReverse(sourceNode, targetNode, [&](fuint32_t adjSourceNode){
    connections.unsafe_erase(adjSourceNode);
    return connections.empty();
  });
  return connections.empty();
}
