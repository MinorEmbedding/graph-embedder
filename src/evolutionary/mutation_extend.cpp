#include "evolutionary/mutation_extend.hpp"

#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_manager.hpp>

#include <sstream>

using namespace majorminer;


MutationExtend::MutationExtend(const EmbeddingState& state, EmbeddingManager& embeddingManager, fuint32_t sourceNode)
  : m_state(state), m_embeddingManager(embeddingManager), m_sourceVertex(sourceNode),
    m_time(m_embeddingManager.getTimestamp()) { }

// use embedding manager only
void MutationExtend::execute()
{
  // std::cout << "1. Extend execute node "<<m_sourceVertex << " to " <<m_extendedTarget << "; improving=" << m_improving << "; contains="
  //           << m_embeddingManager.getRemainingTargetNodes().contains(m_extendedTarget) << std::endl;
  // m_embeddingManager.printRemainingTargetNodes();
  if(!m_improving || !m_embeddingManager.getRemainingTargetNodes().contains(m_extendedTarget)) return;
  // std::cout << "2. Extend execute node "<<m_sourceVertex << " to " <<m_extendedTarget << std::endl;
  //int delta = m_embeddingManager.numberFreeNeighborsNeeded(m_sourceVertex);
  //if (delta <= 0) return;
  // std::cout << "3. Extend execute node "<<m_sourceVertex << " to " <<m_extendedTarget << std::endl;
  double improvement = checkImprovement(m_extendedTarget, m_embeddingManager);

  // std::cout << "4. Extend execute node "<<m_sourceVertex << " improves by " <<improvement << std::endl;
  if (improvement < 0)
  { // adopt mutation
    // std::cout << "Applying extend on " << m_sourceVertex << " towards " << m_extendedTarget << std::endl;
    m_embeddingManager.occupyNode(m_extendedTarget);

    m_embeddingManager.insertMappingPair(m_sourceVertex, m_extendedTarget);

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

double MutationExtend::checkImprovement(fuint32_t extendNode, const EmbeddingBase& base)
{
  const auto& remainingTargetNodes = base.getRemainingTargetNodes();
  double improvement = 0;
  base.iterateTargetGraphAdjacent(extendNode, [&](fuint32_t adjacent){
    if (!remainingTargetNodes.contains(adjacent)) improvement -= 1;
  });
  return improvement;
}

bool MutationExtend::prepare()
{
  int delta = std::max(m_state.numberFreeNeighborsNeeded(m_sourceVertex), 0);
  // std::cout << "Extend - node m_sourceVertex " << m_sourceVertex << " has delta of " << delta << std::endl;
  if (delta == 0) return false;
  const auto& remainingTargetNodes = m_state.getRemainingTargetNodes();

  double bestVal = MAXFLOAT;
  fuint32_t bestExtend = -1;

  nodeset_t candidates{};
  m_state.iterateSourceMappingAdjacent<true>(m_sourceVertex, [&](fuint32_t neighbor, fuint32_t){
    if (remainingTargetNodes.contains(neighbor)) candidates.insert(neighbor);
    return false;
  });

  for (auto& candidate : candidates)
  {
    double improvement = checkImprovement(candidate, m_state);
    if (improvement < bestVal)
    {
      bestVal = improvement;
      bestExtend = candidate;
    }
  }
  m_improving = bestVal < 0;
  m_extendedTarget = bestExtend;

  m_time = m_embeddingManager.getTimestamp();
  return m_improving;
}

bool MutationExtend::isValid()
{
  // std::cout << "Time" << m_time << std::endl;
  // std::cout <<"History "<< m_embeddingManager.getHistory(m_sourceVertex).m_timestampNodeChanged <<std::endl;
  return m_embeddingManager.getHistory(m_sourceVertex).m_timestampNodeChanged < m_time;
}

