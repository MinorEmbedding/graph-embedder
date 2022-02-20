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

bool EmbeddingState::removeRemainingNode(fuint32_t node)
{
  if (!m_nodesRemaining.contains(node)) return false;
  m_nodesRemaining.unsafe_erase(node);
  return true;
}

void EmbeddingState::updateNeededNeighbors(fuint32_t node)
{
  fuint32_t nbNodes = 0;
  iterateSourceGraphAdjacent(node, [&, this](fuint32_t adjacentSource){
    if (isNodeMapped(adjacentSource))
    {
      nbNodes++; m_sourceNeededNeighbors[adjacentSource]--;
    }
  });
  m_sourceNeededNeighbors[node] -= nbNodes;
}


void EmbeddingState::updateConnections(fuint32_t node, PrioNodeQueue& nodesToProcess)
{
  iterateSourceGraphAdjacent(node, [&](fuint32_t adjacent){
    auto findIt = m_nodesRemaining.find(adjacent);
    if (findIt != m_nodesRemaining.end())
    {
      findIt->second += 1; // one of its neighbors is now embedded
      nodesToProcess.push(PrioNode{findIt->first, findIt->second});
    }
  });
  // identifyAffected(node);
}

int EmbeddingState::numberFreeNeighborsNeeded(fuint32_t sourceNode) const
{ // TODO: rework
  // std::cout << "Source node " << sourceNode << " needs " << m_sourceNeededNeighbors[sourceNode].load() << " neighbors and has " << m_sourceFreeNeighbors[sourceNode].load() << std::endl;
  auto it = m_sourceNeededNeighbors.find(sourceNode);
  return 2 * (it == m_sourceNeededNeighbors.end() ? 0 : it->second.load())
    - std::max(getSourceNbFreeNeighbors(sourceNode), 0);
}

int EmbeddingState::getSourceNbFreeNeighbors(fuint32_t sourceNode) const
{
  auto it = m_sourceFreeNeighbors.find(sourceNode);
  return it == m_sourceFreeNeighbors.end() ? 0 : it->second.load();
}


void EmbeddingState::mapNode(fuint32_t source, fuint32_t targetNode)
{
  m_nodesOccupied.insert(targetNode);
  m_mapping.insert(std::make_pair(source, targetNode));
  m_reverseMapping.insert(std::make_pair(targetNode, source));
  m_targetNodesRemaining.unsafe_extract(targetNode);
  removeRemainingNode(source);
}

void EmbeddingState::mapNode(fuint32_t source, const nodeset_t& targets)
{
  for (auto targetNode : targets)
  {
    m_nodesOccupied.insert(targetNode);
    m_mapping.insert(std::make_pair(source, targetNode));
    m_reverseMapping.insert(std::make_pair(targetNode, source));
    m_targetNodesRemaining.unsafe_extract(targetNode);
  }
  removeRemainingNode(source);
}

