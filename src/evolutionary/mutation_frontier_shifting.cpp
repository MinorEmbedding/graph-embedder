#include "evolutionary/mutation_frontier_shifting.hpp"

#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_manager.hpp>

#include <sstream>

#define MAX_CANDIDATES 20

using namespace majorminer;

MutationFrontierShifting::MutationFrontierShifting(const EmbeddingState& state, EmbeddingManager& manager,
  fuint32_t conquerorSource)
  : m_state(state), m_manager(manager), m_conqueror(conquerorSource),
    m_victim((fuint32_t)-1), m_valid(false)
{
  // find a victim node for which conqueror is connected to and
  // all other node is not crucial for victim
  ShiftingCandidates cands = m_manager.getCandidatesFor(conquerorSource);
  if (cands.second.get() == nullptr)
  {
    nodeset_t candidateSet{};
    m_state.iterateSourceMappingAdjacent<false>(m_conqueror, [&](fuint32_t target, fuint32_t){
      m_state.iterateReverseMapping(target, [&](fuint32_t cand){
        if (cand != m_conqueror) candidateSet.insert(cand);
      });
      return candidateSet.size() > MAX_CANDIDATES;
    });
    cands = m_manager.setCandidatesFor(m_conqueror, candidateSet);
  }


  /*auto conquerorRange = data.m_victimConnections.equal_range(conquerorSource);

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
  }*/
}

bool MutationFrontierShifting::isCrucial(fuint32_t /* candidateNode */)
{/*
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
  }*/
  return false;
}

double MutationFrontierShifting::calculateImprovement(fuint32_t /* candidateNode */)
{
  // Two factors play a role:
  // 1. Change in chain length
  // 2. Change in free neighbors
  // TODO: add free neighbor calculation
  fuint32_t victimLength = m_state.getSuperVertexSize(m_victim);
  fuint32_t conquerorLength = m_state.getSuperVertexSize(m_conqueror);
  if (victimLength == 0) return MAXFLOAT;

  return pow(victimLength - 1, 2) + pow(conquerorLength + 1, 2)
        - pow(victimLength, 2) - pow(conquerorLength, 2);
}

void MutationFrontierShifting::execute()
{
 /* auto& data = m_suite.m_frontierData;
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
  */
}
