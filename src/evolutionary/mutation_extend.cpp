#include "evolutionary/mutation_extend.hpp"
#include "majorminer.hpp"


using namespace majorminer;


MutationExtend::MutationExtend(const EmbeddingState& state, EmbeddingManager& embeddingManager, fuint32_t sourceNode)
  : m_state(state), m_embeddingManager(embeddingManager), m_sourceVertex(sourceNode),
    m_time(m_embeddingManager.getTimestamp()) { }

double MutationExtend::checkImprovement(fuint32_t extendNode, fuint32_t sourceNode, int delta, bool useManager)
{ // TODO: refactor
  m_degraded.clear();
  double improvement = 0;
  auto* targetNodesRemaining = &m_state.getRemainingTargetNodes();
  auto* revMapping = &m_state.getReverseMapping();
  if (useManager)
  {
    targetNodesRemaining = &m_embeddingManager.getTargetNodesRemaining();
    revMapping = &m_embeddingManager.getReverseMapping();
  }

  auto adjRange = m_state.getTargetAdjGraph().equal_range(extendNode);
  int nbFree = 0;
  for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
  {
    if (targetNodesRemaining->contains(adj->second)) nbFree++;
    else
    { // already mapped to
      // go over each that was mapped to this node and add to degraded
      auto revIt = revMapping->equal_range(adj->second);
      for (auto rev = revIt.first; rev != revIt.second; ++rev)
      {
        m_degraded.insert(rev->second);
      }
    }
  }
  if (nbFree == 0) return MAXFLOAT;
  improvement = - std::min(delta, nbFree) + 1;
  for (auto deg : m_degraded)
  {
    if (improvement >= 0) return MAXFLOAT;
    if (deg == extendNode) continue;
    else
    {
      int needed = (useManager ?
                m_embeddingManager.numberFreeNeighborsNeeded(deg)
                : m_state.numberFreeNeighborsNeeded(deg));
      if (needed > -1) improvement++;
    }
  }
  return improvement;
}

// use embedding manager only
void MutationExtend::execute()
{
  if(!m_improving || !m_embeddingManager.getTargetNodesRemaining().contains(m_extendedTarget)) return;
  int delta = m_embeddingManager.numberFreeNeighborsNeeded(m_sourceVertex);
  if (delta <= 0) return;
  double improvement = checkImprovement(m_extendedTarget, m_sourceVertex, delta, true);
  std::cout << "Val " << improvement << std::endl;
  if (improvement < 0)
  { // adopt mutation
    m_embeddingManager.occupyNode(m_extendedTarget);
    std::cout << "Occupy" << std::endl;
    m_embeddingManager.insertMappingPair(m_sourceVertex, m_extendedTarget);

    std::cout << "Mapping" << std::endl;
    updateFreeNeighbors();
    std::cout << "FreeNeighbors" << std::endl;
    m_embeddingManager.commit();
    std::cout << "Commit " << std::endl;
    if (m_state.hasVisualizer())
    {
      std::stringstream ss;
      ss << "ExtendMutation applied " << m_sourceVertex
         << " -> { ..., " << m_extendedTarget << " }; improvement: "
         << improvement << std::endl;
      // TODO: disgusting
      const_cast<EmbeddingState*>(&m_state)->getVisualizer()->draw(m_embeddingManager.getMapping(), ss.str().c_str());
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
  auto embRange = m_state.getMapping().equal_range(m_sourceVertex);
  const auto& targetGraph = m_state.getTargetAdjGraph();
  const auto& targetNodesRemaining = m_state.getRemainingTargetNodes();
  for (auto embIt = embRange.first; embIt != embRange.second; ++embIt)
  {
    auto adjRange = targetGraph.equal_range(embIt->second);
    for (auto adjIt = adjRange.first; adjIt != adjRange.second; ++adjIt)
    {
      if (targetNodesRemaining.contains(adjIt->second))
      { m_degraded.insert(adjIt->second); }
    }
  }
  m_embeddingManager.setFreeNeighbors(m_sourceVertex, m_degraded.size());
}

bool MutationExtend::prepare()
{
  // check whether node needs more
  int delta = std::max(m_state.numberFreeNeighborsNeeded(m_sourceVertex), 0);
  if (delta == 0) return false;

  // find adjacent target node with highest number
  auto mapRange = m_state.getMapping().equal_range(m_sourceVertex);

  double bestVal = MAXFLOAT;
  fuint32_t bestExtend = -1;
  fuint32_t adjTargetNode = -1;
  const auto& targetGraph = m_state.getTargetAdjGraph();
  const auto& targetNodesRemaining = m_state.getRemainingTargetNodes();
  
  for (auto targetNode = mapRange.first; targetNode != mapRange.second; ++targetNode)
  { // check nodes adjacent to targetNode as possible candidates
    auto adjRange = targetGraph.equal_range(targetNode->second);
    for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
    {
      if (targetNodesRemaining.contains(adj->second))
      { // legitimate candidate
        auto improvement = checkImprovement(adj->second, m_sourceVertex, delta);
        if (improvement < bestVal)
        {
          bestVal = improvement;
          bestExtend = adj->second;
          adjTargetNode = targetNode->second;
        }
      }
    }
  }

  if (bestVal < 0)
  {
    DEBUG(OUT_S << "Emitting extend task for sourceNode=" << m_sourceVertex
      << " to extend to " << bestExtend << ". Improvement: "
      << bestVal << std::endl;)
    m_targetVertex = adjTargetNode;
    m_extendedTarget = bestExtend;
    m_improving = true;
  }
  std::cout << "Extend " << m_improving << std::endl;
  m_time = m_embeddingManager.getTimestamp();
  return m_improving;
}

bool MutationExtend::isValid()
{
  std::cout << "Time" << m_time << std::endl;
  std::cout <<"History "<< m_embeddingManager.getHistory(m_sourceVertex).m_timestampNodeChanged <<std::endl;
  return m_embeddingManager.getHistory(m_sourceVertex).m_timestampNodeChanged < m_time;
}

