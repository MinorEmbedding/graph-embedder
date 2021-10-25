#include "majorminer.hpp"

using namespace majorminer;


std::vector<int> majorminer::testTBB()
{
  std::vector<int> ret{54,2,6,1,45,1,3,9,7,6,45,77,32,45};
  tbb::parallel_sort(ret.begin(), ret.end());
  return ret;
}

EmbeddingSuite::EmbeddingSuite(const graph_t& source, const graph_t& target)
  : m_sourceGraph(&source), m_targetGraph(&target) {
  convertToAdjacencyList(m_source, source);
  convertToAdjacencyList(m_target, target);
  for (const auto& arc : target)
  {
    m_targetNodesRemaining.insert(arc.first);
    m_targetNodesRemaining.insert(arc.second);
  }
}

embedding_mapping_t EmbeddingSuite::find_embedding()
{
  UnorderedSet<fuint32_t> nodesRemaining{};
  for (const auto& arc : *m_sourceGraph)
  {
    nodesRemaining.insert(arc.first);
    nodesRemaining.insert(arc.second);
  }
  while(!nodesRemaining.empty())
  {
    auto node = *nodesRemaining.begin();
    nodesRemaining.unsafe_erase(node);
    auto adjacentNodes = m_source.equal_range(node);
    fuint32_t nbAdjacent = 0;
    fuint32_t adjacentNode = 0;
    for (auto adjNode = adjacentNodes.first; adjNode != adjacentNodes.second; ++adjNode)
    {
      if (m_mapping.find(adjNode->second) != m_mapping.end())
      {
        adjacentNode = adjNode->second;
        nbAdjacent++;
      }
    }
    DEBUG(OUT_S << "Nb adjacent to node " << node << ": " << nbAdjacent << std::endl;)
    if (nbAdjacent > 1)
    {
      DEBUG(OUT_S << "Complex node to embedd." << std::endl;)
      embeddNode(node);
    }
    else if (nbAdjacent == 1)
    { // find a node that is adjacent to the node "adjacentNode"
      DEBUG(OUT_S << "Simple adjacent node to embedd." << std::endl;)
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
    else
    { // just place the node anywhere
      DEBUG(OUT_S << "Trivial node to embedd." << std::endl;)
      if (!m_targetNodesRemaining.empty())
      {
        auto targetNode = *m_targetNodesRemaining.begin();
        m_targetNodesRemaining.unsafe_erase(m_targetNodesRemaining.begin());
        mapNode(node, targetNode);
      }
      else
      {
        throw std::runtime_error("Oooops...");
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
