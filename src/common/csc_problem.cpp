#include "common/csc_problem.hpp"

#include <common/cut_vertex.hpp>
#include <common/embedding_base.hpp>
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


bool majorminer::isNodeCrucial(const EmbeddingBase& base, fuint32_t sourceNode, fuint32_t targetNode, fuint32_t conqueror)
{
  // 1. Check whether targetNode is crucial due to it being a cut vertex
  // std::cout << "Checking whether cut vertex " << std::endl;
  if (isCutVertex(base, sourceNode, targetNode)) return true;
  // std::cout << "Is no cut vertex" << std::endl;
  // 2. Check whether targetNode is crucial connections to other super vertices
  const auto& mapping = base.getMapping();
  nodeset_t connections{};

  base.iterateSourceGraphAdjacent(sourceNode, [&](fuint32_t adjSourceNode){
    if (mapping.contains(adjSourceNode)) connections.insert(adjSourceNode);
  });
  connections.unsafe_erase(conqueror);

  base.iterateSourceMappingAdjacentReverse(sourceNode, targetNode, [&](fuint32_t adjSourceNode){
    connections.unsafe_erase(adjSourceNode);
    return connections.empty();
  });
  // std::cout << "Connections empty? " << connections.empty() << std::endl;
  return !connections.empty();
}

SuperVertexReducer::SuperVertexReducer(const EmbeddingBase& base, fuint32_t sourceVertex)
  : m_embedding(base), m_sourceVertex(sourceVertex), m_done(false)
{ }

void SuperVertexReducer::initialize(const nodeset_t& currentMapping)
{
  // prepare m_potentialNodes
  m_superVertex.insert(currentMapping.begin(), currentMapping.end());

  m_potentialNodes.insert(m_superVertex.begin(), m_superVertex.end());

  const auto& remaining = m_embedding.getRemainingTargetNodes();
  for (auto mapped : m_superVertex)
  {
    m_embedding.iterateTargetGraphAdjacent(mapped, [&](fuint32_t targetAdj){
      if (remaining.contains(targetAdj)) m_potentialNodes.insert(targetAdj);
    });
  }

  if (m_potentialNodes.size() == m_superVertex.size() || m_potentialNodes.size() <= 1)
  {
    m_done = true;
    return;
  }

  // prepare source vertices in m_sourceConnections
  const auto& mapping = m_embedding.getMapping();
  m_embedding.iterateSourceGraphAdjacent(m_sourceVertex,
    [&](fuint32_t adjacent){
      if (mapping.contains(adjacent)) m_sourceConnections[adjacent] = 0;
  });

  // prepare m_adjacencies
  m_verticesList = std::make_unique<fuint32_t[]>(m_potentialNodes.size());
  UnorderedSet<fuint32_pair_t, PairHashFunc<fuint32_t, fuint32_t>> adjacentSource;
  fuint32_t idx = 0;
  for (auto node : m_potentialNodes)
  {
    m_verticesList[idx++] = node;
    m_embedding.iterateTargetAdjacentReverseMapping(node, [&](fuint32_t adjSource){
      if (m_sourceConnections.contains(adjSource)) adjacentSource.insert(std::make_pair(node, adjSource));
    });
  }
  for (auto& adj : adjacentSource) m_adjacencies.insert(adj);

  // prepare m_sourceConnections
  for (const auto& adj : m_adjacencies)
  {
    if (m_superVertex.contains(adj.first)) m_sourceConnections[adj.second]++;
  }
}

void SuperVertexReducer::optimize()
{
  if (m_done) return;
  RandomGen rand{};
  m_rand = &rand;
  fuint32_t maxIters = 8 * m_potentialNodes.size();
  fuint32_t halfMax = maxIters / 2;
  fuint32_t iteration = 0;
  for (; iteration <  halfMax; ++iteration)
  {
    fuint32_t flipIdx = rand.getRandomUint(m_potentialNodes.size() - 1);
    fuint32_t target = m_verticesList[flipIdx];
    if (m_superVertex.contains(target)) removeNode(target);
    else addNode(target);
  }
  // std::cout << "Removing bad nodes..." << std::endl;
  // printNodeset(m_superVertex);

  for (auto& node : m_potentialNodes)
  {
    if (m_superVertex.contains(node) && isBadNode(node))
    {
      // std::cout << "Bad node " << node << std::endl;
      removeNode(node);
    }
  }
  // std::cout << "Done removing bad nodes. " << std::endl;
  for (; iteration <  maxIters && m_superVertex.size() > 1; ++iteration)
  {
    fuint32_t flipIdx = rand.getRandomUint(m_potentialNodes.size() - 1);
    fuint32_t target = m_verticesList[flipIdx];
    if (m_superVertex.contains(target)) removeNode(target);
  }
  m_done = true;
  printNodeset(m_superVertex);
}

bool SuperVertexReducer::isBadNode(fuint32_t target) const
{
  return m_embedding.getReverseMapping().count(target) >= 1;
}

void SuperVertexReducer::addNode(fuint32_t target)
{
  if (m_superVertex.contains(target) || isBadNode(target) || !isConnected(target)) return;

  m_superVertex.insert(target);
  auto range = m_adjacencies.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    m_sourceConnections[it->second]++;
  }
}

void SuperVertexReducer::removeNode(fuint32_t target)
{
  if (!m_superVertex.contains(target)) return;
  // 1. Check whether for each adjacent source vertex, there is another adjacent target node
  auto range = m_adjacencies.equal_range(target);
  // std::cout << "-----" << std::endl;
  for (auto it = range.first; it != range.second; ++it)
  {
    // std::cout << "Node " << it->second << ": " << m_sourceConnections[it->second] << std::endl;
    if (m_sourceConnections[it->second] <= 1) return;
  }
  // std::cout << "-----" << std::endl;

  // 2. Check whether cut vertex
  nodeset_t temp {};
  temp.insert(m_superVertex.begin(), m_superVertex.end());
  bool isCutV = isCutVertex(m_embedding, temp, target);
  // std::cout << "Node " <<  target << " cutV? " << isCutV << std::endl;
  if (isCutV) return;

  // Now remove the vertex
  m_superVertex.unsafe_erase(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    m_sourceConnections[it->second]--;
  }
}

bool SuperVertexReducer::isConnected(fuint32_t target) const
{
  bool connected = false;
  m_embedding.iterateTargetGraphAdjacentBreak(target, [&](fuint32_t adj){
    connected |= m_superVertex.contains(adj);
    return connected;
  });
  return connected;
}

fuint32_t SuperVertexReducer::checkScore(const nodeset_t& placement) const
{
  fuint32_t numberBad = 0;
  for (auto node : placement) if (isBadNode(node)) numberBad++;
  return numberBad;
}

const nodeset_t& SuperVertexReducer::getBetterPlacement(const nodeset_t& previous) const
{
  fuint32_t scorePrevious = checkScore(previous);
  fuint32_t scoreOwn = checkScore(m_superVertex);
  return (scorePrevious < scoreOwn || (scorePrevious == scoreOwn && previous.size() < m_superVertex.size()))
    ? previous : m_superVertex;
}