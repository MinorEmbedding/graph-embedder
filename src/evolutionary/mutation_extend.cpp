#include "evolutionary/mutation_extend.hpp"

#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_manager.hpp>

#include <sstream>

using namespace majorminer;


MutationExtend::MutationExtend(const EmbeddingState& state, EmbeddingManager& embeddingManager, fuint32_t sourceNode)
  : m_state(state), m_embeddingManager(embeddingManager), m_sourceVertex(sourceNode),
    m_time(m_embeddingManager.getTimestamp()) { }

double MutationExtend::checkImprovement(fuint32_t extendNode, int delta, bool useManager)
{
  m_degraded.clear();

  const EmbeddingBase* base = (useManager ?
      reinterpret_cast<const EmbeddingBase*>(&m_embeddingManager)
      : reinterpret_cast<const EmbeddingBase*>(&m_state));

  int nbFree = 0;
  double improvement = 0;
  auto& degraded = m_degraded;

  base->iterateTargetGraphAdjacent(extendNode,
    [&, this](fuint32_t adjacent){
      if (base->isTargetNodeOccupied(adjacent)) nbFree++;
      else
      { // already mapped to
        // go over each that was mapped to this node and add to degraded
        m_state.iterateReverseMapping(adjacent, [&](fuint32_t sourceNode)
          { degraded.insert(sourceNode); });
      }
  });

  if (nbFree == 0) return MAXFLOAT;
  improvement = - std::min(delta, nbFree) + 1;
  for (auto deg : m_degraded)
  {
    if (improvement >= 0) return MAXFLOAT;
    if (deg == extendNode) continue;
    else
    {
      int needed = base->numberFreeNeighborsNeeded(deg);
      if (needed > -1) improvement++;
    }
  }
  return improvement;
}

// use embedding manager only
void MutationExtend::execute()
{
  std::cout << "---------------------------" << std::endl;
  std::cout << "1. Extend execute node "<<m_sourceVertex << " to " <<m_extendedTarget << "; improving=" << m_improving << "; contains="
            << m_embeddingManager.getRemainingTargetNodes().contains(m_extendedTarget) << std::endl;
  //m_embeddingManager.printRemainingTargetNodes();
  if(!m_improving || !m_embeddingManager.getRemainingTargetNodes().contains(m_extendedTarget)) return;
  std::cout << "2. Extend execute node "<<m_sourceVertex << " to " <<m_extendedTarget << std::endl;
  int delta = m_embeddingManager.numberFreeNeighborsNeeded(m_sourceVertex);
  if (delta <= 0) return;
  std::cout << "3. Extend execute node "<<m_sourceVertex << " to " <<m_extendedTarget << std::endl;
  double improvement = checkImprovement(m_extendedTarget, delta, true);

  std::cout << "4. Extend execute node "<<m_sourceVertex << " improves by " <<improvement << std::endl;
  if (improvement < 0)
  { // adopt mutation
    std::cout << "Applying extend on " << m_sourceVertex << " towards " << m_extendedTarget << std::endl;
    m_embeddingManager.occupyNode(m_extendedTarget);

    m_embeddingManager.insertMappingPair(m_sourceVertex, m_extendedTarget);

    updateFreeNeighbors();
    m_embeddingManager.commit();

    if (m_state.hasVisualizer())
    {
      std::stringstream ss;
      ss << "ExtendMutation applied " << m_sourceVertex
         << " -> { ..., " << m_extendedTarget << " }; improvement: "
         << improvement << std::endl;
      m_embeddingManager.getVisualizer()->draw(m_embeddingManager.getMapping(), ss.str().c_str());
    }
  }
}

void MutationExtend::updateFreeNeighbors()
{
  for (auto deg : m_degraded)
  {
    if (deg != m_sourceVertex) m_embeddingManager.setFreeNeighbors(deg, m_state.getSourceNbFreeNeighbors(deg) - 1);
  }
  m_degraded.clear();
  const auto& targetNodesRemaining = m_state.getRemainingTargetNodes();

  m_state.iterateSourceMappingAdjacent<false>(m_sourceVertex,
    [&](fuint32_t adjNeighbor, fuint32_t){
      if (targetNodesRemaining.contains(adjNeighbor)) m_degraded.insert(adjNeighbor);
      return false;
  });
  m_embeddingManager.setFreeNeighbors(m_sourceVertex, m_degraded.size());
}

bool MutationExtend::prepare()
{
  // check whether node needs more
  int delta = std::max(m_state.numberFreeNeighborsNeeded(m_sourceVertex), 0);
  // std::cout << "Extend - node m_sourceVertex " << m_sourceVertex << " has delta of " << delta << std::endl;
  if (delta == 0) return false;

  double bestVal = MAXFLOAT;
  fuint32_t bestExtend = -1;
  fuint32_t adjTargetNode = -1;

  m_state.iterateSourceMappingAdjacent<true>(m_sourceVertex,
    [&, this](fuint32_t targetNodeExtend, fuint32_t targetNode){
      auto improvement = this->checkImprovement(targetNode, delta);
      if (improvement < bestVal)
      {
        bestVal = improvement;
        bestExtend = targetNodeExtend;
        adjTargetNode = targetNode;
      }
      return false;
  });
  // std::cout << "Extend - BestVal for node " << m_sourceVertex << " is " << bestVal <<std::endl;
  if (bestVal < 0)
  {
    DEBUG(OUT_S << "Emitting extend task for sourceNode=" << m_sourceVertex
      << " to extend to " << bestExtend << ". Improvement: "
      << bestVal << std::endl;)
    m_targetVertex = adjTargetNode;
    m_extendedTarget = bestExtend;
    m_improving = true;
  }

  m_time = m_embeddingManager.getTimestamp();
  return m_improving;
}

bool MutationExtend::isValid()
{
  // std::cout << "Time" << m_time << std::endl;
  // std::cout <<"History "<< m_embeddingManager.getHistory(m_sourceVertex).m_timestampNodeChanged <<std::endl;
  return m_embeddingManager.getHistory(m_sourceVertex).m_timestampNodeChanged < m_time;
}

