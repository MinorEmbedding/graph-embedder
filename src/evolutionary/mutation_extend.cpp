#include "evolutionary/mutation_extend.hpp"
#include "majorminer.hpp"


using namespace majorminer;


MutationExtend::MutationExtend(EmbeddingSuite* suite, fuint32_t sourceNode)
  : m_suite(*suite), m_sourceVertex(sourceNode) { }

double MutationExtend::checkImprovement(fuint32_t extendNode, fuint32_t sourceNode, int delta, bool useManager)
{
  m_degraded.clear();
  double improvement = 0;
  auto& targetNodesRemaining = m_suite.m_targetNodesRemaining;
  auto& revMapping = m_suite.m_reverseMapping;
  if (useManager)
  {
    auto& manager = m_suite.m_embeddingManager;
    targetNodesRemaining = manager.getTargetNodesRemaining();
    revMapping = manager.getReverseMapping();
  }

  auto adjRange = m_suite.m_target.equal_range(extendNode);
  int nbFree = 0;
  for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
  {
    if (targetNodesRemaining.contains(adj->second)) nbFree++;
    else
    { // already mapped to
      // go over each that was mapped to this node and add to degraded
      auto revIt = revMapping.equal_range(adj->second);
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
                m_suite.m_embeddingManager.numberFreeNeighborsNeeded(deg)
                : m_suite.numberFreeNeighborsNeeded(deg));
      if (needed > -1) improvement++;
    }
  }
  return improvement;
}

// use embedding manager only
void MutationExtend::execute()
{
  auto& manager = m_suite.m_embeddingManager;
  if(!m_improving || !manager.getTargetNodesRemaining().contains(m_extendedTarget)) return;
  int delta = manager.numberFreeNeighborsNeeded(m_sourceVertex);
  if (delta <= 0) return;
  double improvement = checkImprovement(m_extendedTarget, m_sourceVertex, delta, true);
  if (improvement < 0)
  { // adopt mutation
    manager.occupyNode(m_extendedTarget);
    manager.insertMappingPair(m_sourceVertex, m_extendedTarget);

    updateFreeNeighbors();
    manager.commit();
    if (m_suite.m_visualizer != nullptr)
    {
      std::stringstream ss;
      ss << "ExtendMutation applied " << m_sourceVertex
         << " -> { ..., " << m_extendedTarget << " }; improvement: "
         << improvement << std::endl;
      m_suite.m_visualizer->draw(manager.getMapping(), ss.str().c_str());
    }
  }
}

void MutationExtend::updateFreeNeighbors()
{
  auto& manager = m_suite.m_embeddingManager;
  for (auto deg : m_degraded)
  {
    if (deg != m_sourceVertex) manager.setFreeNeighbors(deg, m_suite.m_sourceFreeNeighbors[deg] - 1);
  }
  m_degraded.clear();
  auto embRange = m_suite.m_mapping.equal_range(m_sourceVertex);
  for (auto embIt = embRange.first; embIt != embRange.second; ++embIt)
  {
    auto adjRange = m_suite.m_target.equal_range(embIt->second);
    for (auto adjIt = adjRange.first; adjIt != adjRange.second; ++adjIt)
    {
      if (m_suite.m_targetNodesRemaining.contains(adjIt->second))
      { m_degraded.insert(adjIt->second); }
    }
  }
  manager.setFreeNeighbors(m_sourceVertex, m_degraded.size());
}

bool MutationExtend::prepare()
{
  // check whether node needs more
  int delta = std::max(m_suite.numberFreeNeighborsNeeded(m_sourceVertex), 0);
  if (delta == 0) return false;

  // find adjacent target node with highest number
  auto mapRange = m_suite.m_mapping.equal_range(m_sourceVertex);

  double bestVal = MAXFLOAT;
  fuint32_t bestExtend = -1;
  fuint32_t adjTargetNode = -1;

  for (auto targetNode = mapRange.first; targetNode != mapRange.second; ++targetNode)
  { // check nodes adjacent to targetNode as possible candidates
    auto adjRange = m_suite.m_target.equal_range(targetNode->second);
    for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
    {
      if (m_suite.m_targetNodesRemaining.contains(adj->second))
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
  return m_improving;
}

bool MutationExtend::isValid()
{
  return false;
}

