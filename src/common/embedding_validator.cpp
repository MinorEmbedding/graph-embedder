#include "embedding_validator.hpp"

using namespace majorminer;

bool EmbeddingValidator::isDisjoint() const
{
  UnorderedSet<fuint32_t> nodesOccupied{};
  for (const auto& mapped : m_embedding)
  {
    if (nodesOccupied.contains(mapped.second)) return false;
    nodesOccupied.insert(mapped.second);
  }
  return true;
}


bool EmbeddingValidator::nodesConnected() const
{
  UnorderedMultiMap<fuint32_t, fuint32_t> adjacencies{};
  for (const auto& e : m_source)
  {
    adjacencies.insert(orderedPair(e));
  }

  UnorderedMultiMap<fuint32_t, fuint32_t> reverseMapping{};
  for (const auto& mapped : m_embedding)
  {
    reverseMapping.insert(std::make_pair(mapped.second, mapped.first));
  }

  Vector<fuint32_t> mappedNodes{};
  UnorderedMap<fuint32_t, std::atomic<bool>> adjacentNodes{};
  auto groupIt = adjacencies.begin();
  fuint32_t idx;

  while(groupIt != adjacencies.end())
  {
    adjacentNodes.clear();
    auto adjRange = adjacencies.equal_range(groupIt->first);
    for (auto it = adjRange.first; it != adjRange.second; ++it)
    {
      adjacentNodes.insert(std::make_pair(it->second, false));
    }
    auto mappedRange = m_embedding.equal_range(groupIt->first);
    mappedNodes.resize(std::distance(mappedRange.first, mappedRange.second));
    idx = 0;
    for (auto it = mappedRange.first; it != mappedRange.second; ++it)
    {
      mappedNodes[idx++] = it->second;
    }

    // for each node in adjacentNodes search for adjacency to mappedNodes
    tbb::parallel_for_each(mappedNodes.begin(), mappedNodes.end(),
      [&adjacentNodes, &reverseMapping, this](fuint32_t targetNode) {
        auto targetNodeMapped = reverseMapping.equal_range(targetNode);
        for (auto it = targetNodeMapped.first; it != targetNodeMapped.second; ++it)
        {
          if (adjacentNodes.contains(it->second))
          {
            adjacentNodes[it->second] = true;
          }
        }
        auto adjacentIt = this->m_target.equal_range(targetNode);
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
      if (adjNode.second == 0) return false;
    }
    groupIt = adjRange.second;
  }

  return true;

}
