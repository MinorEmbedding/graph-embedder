#include "common/csc_problem.hpp"

#include <common/cut_vertex.hpp>
#include <common/embedding_base.hpp>
#include <common/debug_utils.hpp>

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
  // std::cout << "Checking whether cut vertex " << std::endl;
  if (isCutVertex(base, sourceNode, targetNode)) return true;
  // std::cout << "Is no cut vertex" << std::endl;
  // 1. Check whether targetNode is crucial connections to other super vertices
  nodeset_t sourceConnections{};
  base.iterateSourceGraphAdjacent(sourceNode, [&](fuint32_t adjSourceNode){
    sourceConnections.insert(adjSourceNode);
  });

  nodeset_t connections{};
  base.iterateTargetAdjacentReverseMapping(targetNode, [&](fuint32_t adjSourceNode){
    if (sourceConnections.contains(adjSourceNode)) connections.insert(adjSourceNode);
  });
  // std::cout << "Number connections" << connections.size() << std::endl;

  base.iterateSourceMappingAdjacentReverse(sourceNode, targetNode, [&](fuint32_t adjSourceNode){
    connections.unsafe_erase(adjSourceNode);
    return connections.empty();
  });
  // std::cout << "Connections empty? " << connections.empty() << std::endl;
  return !connections.empty();
}

SuperVertexReducer::SuperVertexReducer(const EmbeddingBase& base, fuint32_t sourceVertex)
  : m_sourceVertex(sourceVertex), m_done(false)
{
  initialize(base);
}

void SuperVertexReducer::initialize(const EmbeddingBase& base)
{
  auto sVertexRange = base.getMapping().equal_range(m_sourceVertex);
  for (auto it = sVertexRange.first; it != sVertexRange.second; ++it)
  {
    m_superVertex.insert(it->second);
  }

  m_potentialNodes.insert(m_superVertex.begin(), m_superVertex.end());

  base.iterateSourceMappingAdjacent<true>(m_sourceVertex,
    [&](fuint32_t targetNode, fuint32_t){
      m_potentialNodes.insert(targetNode); return false;
  });
  if (m_potentialNodes.size() == m_superVertex.size())
  {
    m_done = true;
    return;
  }

  nodeset_t sourceVertices{};
  base.iterateSourceGraphAdjacent(m_sourceVertex,
    [&sourceVertices](fuint32_t adjacent){
      sourceVertices.insert(adjacent);
  });
  for (auto node : m_potentialNodes)
  {
    base.iterateTargetAdjacentReverseMapping(node, [&](fuint32_t adjSource){
      if (sourceVertices.contains(adjSource)) m_adjacencies.insert(std::make_pair(node, adjSource));
    });
  }
}


void SuperVertexReducer::optimize()
{
  std::cout << "Potential nodes: ";
  printNodeset(m_potentialNodes);
  std::cout << std::endl;
}

