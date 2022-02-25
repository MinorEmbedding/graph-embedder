#include "majorminer.hpp"

#include "common/graph_gen.hpp"
#include "common/utils.hpp"
#include "common/embedding_validator.hpp"
#include "common/embedding_visualizer.hpp"
#include "common/cut_vertex.hpp"

#include "evolutionary/mutation_extend.hpp"
#include "evolutionary/mutation_frontier_shifting.hpp"

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
