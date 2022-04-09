#include "embedding_validator.hpp"

#include "common/utils.hpp"
#include "common/embedding_state.hpp"

using namespace majorminer;

bool EmbeddingValidator::isDisjoint() const
{
  UnorderedSet<fuint32_t> nodesOccupied{};
  const auto& embedding = m_state.getMapping();
  for (const auto& mapped : embedding)
  {
    if (nodesOccupied.contains(mapped.second))
    {
      DEBUG(OUT_S << "Not an injective mapping." << std::endl;)
      return false;
    }
    nodesOccupied.insert(mapped.second);
  }
  return true;
}


bool EmbeddingValidator::nodesConnected() const
{
  const auto& embedding = m_state.getMapping();
  const auto& sourceGraph = *m_state.getSourceGraph();
  UnorderedMultiMap<fuint32_t, fuint32_t> adjacencies{};
  for (const auto& e : sourceGraph)
  {
    adjacencies.insert(orderedPair(e));
  }

  UnorderedMap<fuint32_t, std::atomic<bool>> adjacentNodes{};
  auto groupIt = adjacencies.begin();

  while(groupIt != adjacencies.end())
  {
    adjacentNodes.clear();
    auto adjRange = adjacencies.equal_range(groupIt->first);
    for (auto it = adjRange.first; it != adjRange.second; ++it)
    {
      adjacentNodes.insert(std::make_pair(it->second, false));
    }
    auto mappedRange = embedding.equal_range(groupIt->first);

    // for each node in adjacentNodes search for adjacency to mappedNodes
    tbb::parallel_for_each(mappedRange.first, mappedRange.second,
      [&](fuint32_pair_t targetNodeP) {
        m_state.iterateReverseMapping(targetNodeP.second, [&](fuint32_t reverse){
          if (adjacentNodes.contains(reverse)) adjacentNodes[reverse] = true;
        });

        m_state.iterateTargetAdjacentReverseMapping(targetNodeP.second, [&](fuint32_t revSourceNode){
          if (adjacentNodes.contains(revSourceNode)) adjacentNodes[revSourceNode] = true;
        });
    });

    for (const auto& adjNode : adjacentNodes)
    {
      if (!adjNode.second)
      {
        DEBUG(OUT_S << "Not all edges embedded! Node " << adjNode.first << " is missing at least one edge." << std::endl;)
        DEBUG(printMissingEdges(adjNode.first));
        return false;
      }
    }
    groupIt = adjRange.second;
  }

  return true;

}

void EmbeddingValidator::printMissingEdges(fuint32_t node) const
{
  nodeset_t missingAdjacent{};

  m_state.iterateSourceGraphAdjacent(node, [&missingAdjacent](fuint32_t adjacentSource)
    { missingAdjacent.insert(adjacentSource); });

  if (missingAdjacent.empty()) return;

  m_state.iterateSourceMappingAdjacent<false>(node, [&](fuint32_t adjacent, fuint32_t){
    m_state.iterateReverseMapping(adjacent, [&](fuint32_t reverse){
      missingAdjacent.unsafe_erase(reverse);
    });
    return false;
  });

  OUT_S << "Node " << node << " is missing edges to nodes { ";
  for (auto missing : missingAdjacent) OUT_S << missing << " ";
  OUT_S << "}" << std::endl;
}
