#include "embedding_analyzer.hpp"

using namespace majorminer;

fuint32_t EmbeddingAnalyzer::getNbOverlaps() const
{
  UnorderedMap<fuint32_t, fuint32_t> overlaps{};
  for(const auto& p : m_embedding)
  {
    overlaps[p.second]++;
  }
  fuint32_t nbOverlaps = 0;
  for (const auto& p : overlaps)
  {
    if (p.second > 1) nbOverlaps += p.second;
  }
  return nbOverlaps;
}

fuint32_t EmbeddingAnalyzer::getNbUsedNodes() const
{
  nodeset_t targetNodes{};
  tbb::parallel_for_each(m_embedding.begin(), m_embedding.end(),
      [&](const edge_t& p){
        targetNodes.insert(p.second);
  });
  return targetNodes.size();
}