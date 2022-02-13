#include "mutation_extend.hpp"

using namespace majorminer;


MutationExtend::MutationExtend(EmbeddingSuite* suite, fuint32_t sourceNode)
  : m_suite(*suite) {
  // check whether node needs more
  int delta = std::max(suite->numberFreeNeighborsNeeded(sourceNode), 0);
  if (delta == 0) return;

  // find adjacent target node with highest number
  auto mapRange = suite->m_mapping.equal_range(sourceNode);

  double bestVal = MAXFLOAT;
  fuint32_t bestExtend = -1;
  fuint32_t adjTargetNode = -1;

  for (auto targetNode = mapRange.first; targetNode != mapRange.second; ++targetNode)
  { // check nodes adjacent to targetNode as possible candidates
    auto adjRange = suite->m_target.equal_range(targetNode->second);
    for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
    {
      if (m_suite.m_targetNodesRemaining.contains(adj->second))
      { // legitimate candidate
        auto improvement = checkCandidate(adj->second, sourceNode, delta);
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
    DEBUG(OUT_S << "Emitting extend task for sourceNode=" << sourceNode
      << " to extend to " << bestExtend << ". Improvement: "
      << bestVal << std::endl;)
    m_sourceVertex = sourceNode;
    m_targetVertex = adjTargetNode;
    m_extendedTarget = bestExtend;
    m_valid = true;
  }
}

double MutationExtend::checkCandidate(fuint32_t extendNode, fuint32_t sourceNode, int delta)
{
  m_degraded.clear();
  double improvement = 0;

  auto adjRange = m_suite.m_target.equal_range(extendNode);
  int nbFree = 0;
  for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
  {
    if (m_suite.m_targetNodesRemaining.contains(adj->second)) nbFree++;
    else
    { // already mapped to
      // go over each that was mapped to this node and add to degraded
      auto revIt = m_suite.m_reverseMapping.equal_range(adj->second);
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
      int needed = m_suite.numberFreeNeighborsNeeded(deg);
      if (needed > -1) improvement++;
    }
  }
  return improvement;
}

void MutationExtend::execute()
{
  if(!m_valid || !m_suite.m_targetNodesRemaining.contains(m_extendedTarget)) return;
  int delta = m_suite.numberFreeNeighborsNeeded(m_sourceVertex);
  if (delta <= 0) return;
  double improvement = checkCandidate(m_extendedTarget, m_sourceVertex, delta);
  if (improvement < 0)
  { // adopt mutation
    m_suite.m_targetNodesRemaining.unsafe_erase(m_extendedTarget);
    m_suite.m_nodesOccupied.insert(m_extendedTarget);
    m_suite.m_mapping.insert(std::make_pair(m_sourceVertex, m_extendedTarget));
    m_suite.m_reverseMapping.insert(std::make_pair(m_extendedTarget, m_sourceVertex));

    for (auto deg : m_degraded)
    {
      if (deg != m_sourceVertex) m_suite.m_sourceFreeNeighbors[deg]--;
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
    m_suite.m_sourceFreeNeighbors[m_sourceVertex] = m_degraded.size();

    if (m_suite.m_visualizer != nullptr)
    {
      std::stringstream ss;
      ss << "ExtendMutation applied " << m_sourceVertex
         << " -> { ..., " << m_extendedTarget << " }; improvement: "
         << improvement << std::endl;
      m_suite.m_visualizer->draw(m_suite.m_mapping, ss.str().c_str());
    }
  }
}
