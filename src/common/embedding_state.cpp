#include "embedding_state.hpp"

#include "common/utils.hpp"

using namespace majorminer;

EmbeddingState::EmbeddingState(const graph_t& sourceGraph, const graph_t& targetGraph, EmbeddingVisualizer* vis)
  : m_sourceGraph(&sourceGraph), m_targetGraph(&targetGraph), m_visualizer(vis)
{
  initialize();
}

void EmbeddingState::initialize()
{
  convertToAdjacencyList(m_source, *m_sourceGraph);
  convertToAdjacencyList(m_target, *m_targetGraph);
  for (const auto& arc : *m_targetGraph)
  {
    m_targetNodesRemaining.insert(arc.first);
    m_targetNodesRemaining.insert(arc.second);
  }
  for (const auto& arc : *m_sourceGraph)
  {
    m_nodesRemaining[arc.first] = 0;
    m_nodesRemaining[arc.second] = 0;
    m_sourceNeededNeighbors[arc.first]++;
    m_sourceNeededNeighbors[arc.second]++;
  }
}

fuint32_t EmbeddingState::getTrivialNode()
{ // TODO: assert
  auto node = *m_nodesRemaining.begin();
  m_nodesRemaining.unsafe_erase(m_nodesRemaining.begin());
  return node.first;
}

void EmbeddingState::mapNode(fuint32_t source, const nodeset_t& targets)
{

}

bool EmbeddingState::removeRemainingNode(fuint32_t node)
{
  if (!m_nodesRemaining.contains(node)) return false;
  m_nodesRemaining.unsafe_erase(node);
  return true;
}

void EmbeddingState::updateNeededNeighbors(fuint32_t node)
{
  auto range = m_source.equal_range(node);
  fuint32_t nbNodes = 0;
  for (auto it = range.first; it != range.second; ++it)
  {
    if (!m_nodesRemaining.contains(it->second))
    {
      nbNodes++;
      m_sourceNeededNeighbors[it->second]--;
    }
  }
  m_sourceNeededNeighbors[node] -= nbNodes;
}


void EmbeddingState::updateConnections(fuint32_t node, PrioNodeQueue& nodesToProcess)
{
  auto adjacentRange = m_source.equal_range(node);
  for (auto adjacentIt = adjacentRange.first; adjacentIt != adjacentRange.second; ++adjacentIt)
  {
    auto findIt = m_nodesRemaining.find(adjacentIt->second);
    if (findIt != m_nodesRemaining.end())
    {
      findIt->second += 1; // one of its neighbors is now embedded
      nodesToProcess.push(PrioNode{findIt->first, findIt->second});
    }
  }
  // identifyAffected(node);
}

int EmbeddingState::numberFreeNeighborsNeeded(fuint32_t sourceNode)
{ // TODO: rework
  // std::cout << "Source node " << sourceNode << " needs " << m_sourceNeededNeighbors[sourceNode].load() << " neighbors and has " << m_sourceFreeNeighbors[sourceNode].load() << std::endl;
  return 2 * m_sourceNeededNeighbors[sourceNode].load()
    - std::max(m_sourceFreeNeighbors[sourceNode].load(), 0);
}
