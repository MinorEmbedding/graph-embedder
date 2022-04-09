#include "common/csc_problem.hpp"

#include <common/cut_vertex.hpp>
#include <common/embedding_base.hpp>
#include <common/embedding_manager.hpp>
#include <common/debug_utils.hpp>
#include <common/random_gen.hpp>

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

nodeset_t majorminer::getEmbeddedAdjacentSourceVertices(const EmbeddingBase& base, fuint32_t sourceVertex)
{
  const auto& mapping = base.getMapping();
  nodeset_t connections{};

  base.iterateSourceGraphAdjacent(sourceVertex, [&](fuint32_t adjSourceNode){
    if (mapping.contains(adjSourceNode)) connections.insert(adjSourceNode);
  });
  return connections;
}

bool majorminer::isNodeCrucial(const EmbeddingBase& base, fuint32_t sourceNode, fuint32_t targetNode, fuint32_t conqueror)
{
  // 1. Check whether targetNode is crucial due to it being a cut vertex
  // std::cout << "Checking whether cut vertex " << std::endl;
  if (isCutVertex(base, sourceNode, targetNode)) return true;
  // std::cout << "Is no cut vertex" << std::endl;
  // 2. Check whether targetNode is crucial connections to other super vertices
  nodeset_t connections = getEmbeddedAdjacentSourceVertices(base, sourceNode);
  connections.unsafe_erase(conqueror);

  base.iterateSourceMappingAdjacentReverse(sourceNode, targetNode, [&](fuint32_t adjSourceNode){
    connections.unsafe_erase(adjSourceNode);
    return connections.empty();
  });
  // std::cout << "Connections empty? " << connections.empty() << std::endl;
  return !connections.empty();
}


bool majorminer::connectsToAdjacentVertices(const EmbeddingManager& base,
    const nodeset_t& placement, fuint32_t sourceVertex)
{
  nodeset_t connections = getEmbeddedAdjacentSourceVertices(base, sourceVertex);

  for (auto target : placement)
  {
    base.iterateTargetAdjacentReverseMapping(target,
      [&connections](fuint32_t sourceAdj){
        connections.unsafe_erase(sourceAdj);
    });
  }
  return connections.empty();
}
