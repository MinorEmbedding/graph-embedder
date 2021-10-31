
#include "evolutionary/mutation_frontier_shifting.hpp"

using namespace majorminer;



MuationFrontierShifting::MuationFrontierShifting(EmbeddingSuite* suite,
  fuint32_t conquerorSource, fuint32_t victimSource)
  : m_suite(*suite), m_conqueror(conquerorSource), m_victim(victimSource)
{
  // find a victim node for which conqueror is connected to and
  // all other node is not crucial for victim
  auto conquerorRange = m_suite.m_victimConnections.equal_range(conquerorSource);
  for (auto candidateIt = conquerorRange.first; candidateIt != conquerorRange.second; ++candidateIt)
  {
    if (m_suite.m_cutVertices.contains(candidateIt->second) || isCrucial(candidateIt->second)) continue;
    double val = calculateImprovement(candidateIt->second);
    if (val < m_bestImprovement)
    {
      m_bestImprovement = val;
      m_valid = true;
      m_bestContested = candidateIt->second;
    }
  }
}

bool MuationFrontierShifting::isCrucial(fuint32_t candidateNode)
{
  auto revRange = m_suite.m_reverseConnections.equal_range(candidateNode);
  for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
  { // for every node, we have to make sure that the source node has another
    // connection to the victim chain
    if (revIt->second == m_conqueror) continue;
    bool replaceable = false;
    auto connectionRange = m_suite.m_victimConnections.equal_range(revIt->second);
    for (auto it = connectionRange.first; it != connectionRange.second; ++it)
    {
      if (it->second != candidateNode)
      {
        replaceable = true;
        break;
      }
    }
    if (!replaceable) return true;
  }
  return false;
}

double MuationFrontierShifting::calculateImprovement(fuint32_t candidateNode)
{
  // Two factors play a role:
  // 1. Change in chain length
  // 2. Change in free neighbors
  // TODO: add free neighbor calculation
  fuint32_t victimLength = m_suite.m_mapping.count(m_victim);
  fuint32_t conquerorLength = m_suite.m_mapping.count(m_conqueror);
  if (victimLength == 0) return MAXFLOAT;

  return pow(victimLength - 1, 2) + pow(conquerorLength + 1, 2)
        - pow(victimLength, 2) - pow(conquerorLength, 2);
}

void MuationFrontierShifting::execute()
{
  if (m_valid) std::cout << "Mutation FrontierShifting: " << m_conqueror << " victim " << m_victim << " Contested: " << m_bestContested << std::endl;
  if (!m_valid || isCrucial(m_bestContested) || calculateImprovement(m_bestContested) > 0)
  {

  }
}