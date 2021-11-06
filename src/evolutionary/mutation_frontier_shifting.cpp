
#include "evolutionary/mutation_frontier_shifting.hpp"
#include "majorminer.hpp"

using namespace majorminer;



MuationFrontierShifting::MuationFrontierShifting(EmbeddingSuite* suite,
  fuint32_t conquerorSource, fuint32_t victimSource)
  : m_suite(*suite), m_conqueror(conquerorSource),
    m_victim(victimSource), m_valid(false)
{
  // find a victim node for which conqueror is connected to and
  // all other node is not crucial for victim
  auto& data = m_suite.m_frontierData;
  auto conquerorRange = data.m_victimConnections.equal_range(conquerorSource);

  for (auto candidateIt = conquerorRange.first; candidateIt != conquerorRange.second; ++candidateIt)
  {
    if (data.m_cutVertices.contains(candidateIt->second) || isCrucial(candidateIt->second)) continue;
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
  auto& data = m_suite.m_frontierData;
  auto revRange = data.m_reverseConnections.equal_range(candidateNode);
  for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
  { // for every node, we have to make sure that the source node has another
    // connection to the victim chain
    if (revIt->second == m_conqueror) continue;
    bool replaceable = false;
    auto connectionRange = data.m_victimConnections.equal_range(revIt->second);
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
  auto& data = m_suite.m_frontierData;
  if (!m_valid || isCrucial(m_bestContested)) return;
  if (data.isNowACutVertex(m_bestContested) || calculateImprovement(m_bestContested) >= 0) return;

  // Commit the mutation
  eraseSinglePair(m_suite.m_mapping, m_victim, m_bestContested);
  eraseSinglePair(m_suite.m_reverseMapping, m_bestContested, m_victim);
  m_suite.m_mapping.insert(fuint32_pair_t{ m_conqueror, m_bestContested });
  m_suite.m_reverseMapping.insert(fuint32_pair_t{ m_bestContested, m_conqueror });

  // take care of free neighbors and update frontierData
  m_suite.m_frontierData.lostNode(m_bestContested, m_conqueror);
  if (m_suite.m_visualizer != nullptr)
  {
    m_suite.m_visualizer->draw(m_suite.m_mapping, [this](std::ostream& svg)
    {
      svg << "FrontierShifting. Conqueror "
          << this->getConqueror() << ", victim "
          << this->getVictim() << ", contested "
          << this->getContested();
    });
  }
}
