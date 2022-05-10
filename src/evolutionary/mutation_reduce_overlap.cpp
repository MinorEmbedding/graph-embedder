#include "mutation_reduce_overlap.hpp"

#include <sstream>

#include <common/embedding_state.hpp>
#include <common/embedding_manager.hpp>
#include <common/embedding_visualizer.hpp>
#include <initial/super_vertex_reducer.hpp>

using namespace majorminer;

MutationReduceOverlap::MutationReduceOverlap(EmbeddingState& state,
  EmbeddingManager& manager, fuint32_t sourceVertex)
        : m_state(state), m_manager(manager), m_sourceVertex(sourceVertex)
{
  m_reducer = new SuperVertexReducer{ m_state, sourceVertex };
}

MutationReduceOverlap::~MutationReduceOverlap()
{
  if (m_reducer != nullptr) delete m_reducer;
}

bool MutationReduceOverlap::isValid()
{
  // std::cout << "Checking validity 0x" << ((void*)m_reducer) << std::endl;
  // bool valid = m_reducer->remainsValid(m_manager);
  // std::cout << "Is reduce overlap valid?."  << valid << std::endl;
  return m_reducer->remainsValid(m_manager);
}

bool MutationReduceOverlap::prepare()
{
  m_requeues--;
  m_reducer->initialize();
  m_reducer->optimize();
  // std::cout << "Has overlap improved. " << improved  << std::endl;
  return m_reducer->improved();
}

void MutationReduceOverlap::execute()
{
  // std::cout << "Trying to reduce overlap." << std::endl;
  const auto& initial = m_reducer->getInitialSuperVertex();
  const auto& improved = m_reducer->getSuperVertex();
  const auto& reverse = m_manager.getReverseMapping();

  for (auto target : initial)
  {
    if (!improved.contains(target))
    {
      m_manager.deleteMappingPair(m_sourceVertex, target);
      if (reverse.count(target) <= 1) m_manager.freeNode(target);
    }
  }
  for (auto target : improved)
  {
    if (!initial.contains(target))
    {
      m_manager.insertMappingPair(m_sourceVertex, target);
      m_manager.occupyNode(target);
    }
  }
  m_manager.commit();
  if (m_state.hasVisualizer())
    {
      std::stringstream ss;
      ss << "ReduceOverlap applied on " << m_sourceVertex << "." << std::endl;
      m_manager.getVisualizer()->draw(m_manager.getMapping(), ss.str().c_str());
    }
}

