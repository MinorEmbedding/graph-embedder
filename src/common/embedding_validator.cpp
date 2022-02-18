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
  const auto& targetGraph = m_state.getTargetAdjGraph();
  UnorderedMultiMap<fuint32_t, fuint32_t> adjacencies{};
  for (const auto& e : sourceGraph)
  {
    adjacencies.insert(orderedPair(e));
  }

  UnorderedMultiMap<fuint32_t, fuint32_t> reverseMapping{};
  for (const auto& mapped : embedding)
  {
    reverseMapping.insert(std::make_pair(mapped.second, mapped.first));
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
      [&adjacentNodes, &reverseMapping, targetGraph](fuint32_pair_t targetNodeP) {
        auto targetNodeMapped = reverseMapping.equal_range(targetNodeP.second);
        for (auto it = targetNodeMapped.first; it != targetNodeMapped.second; ++it)
        {
          if (adjacentNodes.contains(it->second))
          {
            adjacentNodes[it->second] = true;
          }
        }
        auto adjacentIt = targetGraph.equal_range(targetNodeP.second);
        for (auto it = adjacentIt.first; it != adjacentIt.second; ++it)
        {
          auto revMappedIt = reverseMapping.equal_range(it->second);
          for (auto revIt = revMappedIt.first; revIt != revMappedIt.second; ++revIt)
          {
            if (adjacentNodes.contains(revIt->second))
            {
              adjacentNodes[revIt->second] = true;
            }
          }
        }
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
  const auto& reverseMapping = m_state.getReverseMapping();
  const auto& sourceGraph = m_state.getSourceAdjGraph();
  const auto& targetGraph = m_state.getTargetAdjGraph();

  auto adjRange = sourceGraph.equal_range(node);
  for (auto it = adjRange.first; it != adjRange.second; ++it) missingAdjacent.insert(it->second);

  if (missingAdjacent.empty()) return;
  auto mappedRange = m_state.getMapping().equal_range(node);
  for (auto mappedIt = mappedRange.first; mappedIt != mappedRange.second; ++mappedIt)
  {
    auto targetAdjRange = targetGraph.equal_range(mappedIt->second);
    for (auto targetAdj = targetAdjRange.first; targetAdj != targetAdjRange.second; ++targetAdj)
    {
      auto revRange = reverseMapping.equal_range(targetAdj->second);
      for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
      {
        missingAdjacent.unsafe_erase(revIt->second);
      }
    }
  }

  OUT_S << "Node " << node << " is missing edges to nodes { ";
  for (auto missing : missingAdjacent) OUT_S << missing << " ";
  OUT_S << "}" << std::endl;
}
