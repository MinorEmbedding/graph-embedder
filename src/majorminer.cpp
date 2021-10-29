#include "majorminer.hpp"

using namespace majorminer;

EmbeddingSuite::EmbeddingSuite(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer)
  : m_sourceGraph(&source), m_targetGraph(&target), m_visualizer(visualizer) {
  convertToAdjacencyList(m_source, source);
  convertToAdjacencyList(m_target, target);
  for (const auto& arc : target)
  {
    m_targetNodesRemaining.insert(arc.first);
    m_targetNodesRemaining.insert(arc.second);
  }
  for (const auto& arc : *m_sourceGraph)
  {
    m_nodesRemaining[arc.first] = 0;
    m_nodesRemaining[arc.second] = 0;
  }
}

embedding_mapping_t EmbeddingSuite::find_embedding()
{
  while(!m_nodesRemaining.empty())
  {
    if (!m_nodesToProcess.empty())
    {
      PrioNode node{};
      auto found = m_nodesToProcess.try_pop(node);
      if (!found) continue;
      if (!m_nodesRemaining.contains(node.m_id)) continue;
      m_nodesRemaining.unsafe_erase(node.m_id);
      if (node.m_nbConnections > 1)
      {
        DEBUG(OUT_S << "Complex node to embedd: " << node.m_id << " (" << node.m_nbConnections << ")" << std::endl;)
        embeddNode(node.m_id);
        if (m_visualizer != nullptr)
        {
          m_visualizer->draw(m_mapping, [&](std::ostream& os)
            { os << "Complex adjacent node " << node.m_id << " (" << node.m_nbConnections << ")";}
          );
        }
      }
      else
      { // nbConnections = 1
        DEBUG(OUT_S << "Simple adjacent node to embedd: " << node.m_id << std::endl;)
        embeddSimpleNode(node.m_id);
        if (m_visualizer != nullptr)
        {
          m_visualizer->draw(m_mapping, [&](std::ostream& os)
            { os << "Simple adjacent node " << node.m_id << " (1).";}
          );
        }
      }
      updateConnections(node.m_id);
    }
    else
    {
      // just pick an arbitrary node and place it somewhere
      auto node = *m_nodesRemaining.begin();
      m_nodesRemaining.unsafe_erase(m_nodesRemaining.begin());
      DEBUG(OUT_S << "Trivial node to embedd: " << node.first << std::endl;)
      embeddTrivialNode(node.first);
      if (m_visualizer != nullptr)
      {
        m_visualizer->draw(m_mapping, [&](std::ostream& os)
            { os << "Trivial node " << node.first;}
        );
      }
    }
  }
  return m_mapping;
}


void EmbeddingSuite::embeddNode(fuint32_t node)
{
  embeddNodeNetworkSimplex(node);
}


void EmbeddingSuite::embeddNodeNetworkSimplex(fuint32_t node)
{
  if (m_nsWrapper.get() == nullptr) m_nsWrapper = std::make_unique<NetworkSimplexWrapper>(this);

  m_nsWrapper->embeddNode(node);
}


void EmbeddingSuite::mapNode(fuint32_t node, fuint32_t targetNode)
{
  DEBUG(std::cout << node << " -> " << targetNode << std::endl;)
  m_nodesOccupied.insert(targetNode);
  m_mapping.insert(std::make_pair(node, targetNode));
  m_targetNodesRemaining.unsafe_extract(targetNode);
}

void EmbeddingSuite::mapNode(fuint32_t node, const nodeset_t& targetNodes)
{
  DEBUG(OUT_S << node << " -> {";)
  for(auto targetNode : targetNodes)
  {
    DEBUG(OUT_S << " " << targetNode;)
    m_nodesOccupied.insert(targetNode);
    m_mapping.insert(std::make_pair(node, targetNode));
    m_targetNodesRemaining.unsafe_extract(targetNode);
  }
  DEBUG(OUT_S << " }" << std::endl;)
}

void EmbeddingSuite::updateConnections(fuint32_t node)
{
  auto adjacentRange = m_source.equal_range(node);
  for (auto adjacentIt = adjacentRange.first; adjacentIt != adjacentRange.second; ++adjacentIt)
  {
    auto findIt = m_nodesRemaining.find(adjacentIt->second);
    if (findIt != m_nodesRemaining.end())
    {
      findIt->second += 1; // one of its neighbors is now embedded
      m_nodesToProcess.push(PrioNode{findIt->first, findIt->second});
    }
  }
}

void EmbeddingSuite::embeddSimpleNode(fuint32_t node)
{
  // find a node that is adjacent to the node "adjacentNode"
  auto adjacentIt = m_source.equal_range(node);
  fuint32_t adjacentNode = -1;
  for (auto n = adjacentIt.first; n != adjacentIt.second; ++n)
  {
    if (!m_nodesRemaining.contains(n->second))
    {
      adjacentNode = n->second;
      break;
    }
  }
  if (adjacentNode == (fuint32_t)-1) throw std::runtime_error("Could not find the adjacent node.");

  auto embeddedPathIt = m_mapping.equal_range(adjacentNode);
  fuint32_t bestNodeFound = adjacentNode;

  for (auto targetNode = embeddedPathIt.first; targetNode != embeddedPathIt.second && bestNodeFound == adjacentNode; ++targetNode)
  {
    // find nodes that are adjacent to targetNode (in the targetGraph)
    auto targetGraphAdjacentIt = m_target.equal_range(targetNode->second);
    for (auto targetAdjacent = targetGraphAdjacentIt.first; targetAdjacent != targetGraphAdjacentIt.second; ++targetAdjacent)
    {
      if (!m_nodesOccupied.contains(targetAdjacent->second))
      {
        bestNodeFound = targetAdjacent->second;
        break;
      }
    }
  }
  // map "node" to "bestNodeFound"
  mapNode(node, bestNodeFound);

}

void EmbeddingSuite::embeddTrivialNode(fuint32_t node)
{
  if (!m_targetNodesRemaining.empty())
  {
    auto targetNode = *m_targetNodesRemaining.begin();
    m_targetNodesRemaining.unsafe_erase(m_targetNodesRemaining.begin());
    mapNode(node, targetNode);
    updateConnections(node);
  }
  else
  {
    throw std::runtime_error("Oooops...");
  }
}

bool EmbeddingSuite::isValid() const
{
  EmbeddingValidator validator{m_mapping, *m_sourceGraph, m_target};
  return validator.isValid();
}


bool EmbeddingSuite::connectsNodes() const
{
  EmbeddingValidator validator{m_mapping, *m_sourceGraph, m_target};
  return validator.nodesConnected();
}
