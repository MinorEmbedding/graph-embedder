#include "majorminer.hpp"

#include <common/graph_gen.hpp>
#include <common/utils.hpp>
#include <common/embedding_validator.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/cut_vertex.hpp>
#include <common/time_measurement.hpp>

#include <evolutionary/mutation_extend.hpp>
#include <evolutionary/mutation_frontier_shifting.hpp>

using namespace majorminer;

EmbeddingSuite::EmbeddingSuite(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer)
  : m_state(source, target, visualizer), m_visualizer(visualizer),
    m_embeddingManager(*this, m_state), m_mutationManager(m_state, m_embeddingManager),
    m_placer(m_state, m_embeddingManager)
{ }

embedding_mapping_t EmbeddingSuite::find_embedding()
{
  const auto& nodesRemaining = m_state.getRemainingNodes();
  while(!nodesRemaining.empty())
  {
    m_placer();
    m_mutationManager();
  }
  m_mutationManager(true);
  m_placer.replaceOverlapping();
  if (m_visualizer != nullptr) finishVisualization();
  PRINT_TIME(NETWORK_SIMPLEX_BUILDTIME)
  PRINT_TIME(NETWORK_SIMPLEX_SOLVETIME)
  return m_state.getMapping();
}


bool EmbeddingSuite::isValid() const
{
  EmbeddingValidator validator{m_state};
  return validator.isValid();
}


bool EmbeddingSuite::connectsNodes() const
{
  EmbeddingValidator validator{m_state};
  return validator.nodesConnected();
}

void EmbeddingSuite::finishVisualization()
{
  fuint32_pair_t stats = calculateOverlappingStats(m_state);
  std::stringstream ss;
  ss  << "Final iteration. Distinct overlaps: " << stats.first
      << "; Total overlaps: " << stats.second << std::endl;
  m_visualizer->draw(m_state.getMapping(), ss.str().c_str());
}
