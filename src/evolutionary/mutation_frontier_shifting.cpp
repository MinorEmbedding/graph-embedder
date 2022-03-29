#include "evolutionary/mutation_frontier_shifting.hpp"

#include <common/utils.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_manager.hpp>
#include <common/csc_problem.hpp>

#include <sstream>

#define MAX_CANDIDATES 20

using namespace majorminer;

namespace
{
  bool isCandidateValid(const ShiftingCandidates& candidate)
  {
    return candidate.second.get() != nullptr;
  }
}

MutationFrontierShifting::MutationFrontierShifting(const EmbeddingState& state, EmbeddingManager& manager,
  fuint32_t conquerorSource)
  : m_state(state), m_manager(manager), m_conqueror(conquerorSource),
    m_victim(FUINT32_UNDEF), m_valid(false)
{ }

bool MutationFrontierShifting::isValid()
{
  // std::cout << "Valid=" << m_valid << "; bestContested=" << m_bestContested << "; improvement=" << calculateImprovement(m_victim)
  //    << "; crucial=" << !isNodeCrucial(m_manager, m_victim, m_bestContested, m_conqueror) << std::endl;
  // std::cout << "Victim=" << m_victim << "; Conqueror=" << m_conqueror << std::endl;
  //char c=getchar();
  //if (c=='E') return false;
  // std::cout << "------------------------------" << std::endl;
  return m_valid && isDefined(m_bestContested) && calculateImprovement(m_victim) < 0
        && !isNodeCrucial(m_manager, m_victim, m_bestContested, m_conqueror);
}

bool MutationFrontierShifting::prepare()
{
  // find a victim node for which conqueror is connected to and
  // all other node is not crucial for victim
  // std::cout << "Preparing shifting. " << m_conqueror << std::endl;
  m_valid = false;
  ShiftingCandidates cands = m_manager.getCandidatesFor(m_conqueror);
  if (!isCandidateValid(cands) || true )
  {
    nodepairset_t candidateSet{};
    m_state.iterateSourceMappingAdjacent<false>(m_conqueror, [&](fuint32_t target, fuint32_t){
      m_state.iterateReverseMapping(target, [&](fuint32_t cand){
        if (cand != m_conqueror) candidateSet.insert(fuint32_pair_t{cand, target});
      });
      return candidateSet.size() > MAX_CANDIDATES;
    });
    cands = m_manager.setCandidatesFor(m_conqueror, candidateSet);
  }

  if (!isCandidateValid(cands)) return false;
  fuint32_pair_t* candidates = cands.second.get();
  auto subgraph = extractSubgraph(m_state, m_conqueror);

  for (fuint32_t idx = 0; idx < cands.first; ++idx)
  {
    fuint32_pair_t candidate = candidates[idx];
    candidates[idx] = std::make_pair(FUINT32_UNDEF, FUINT32_UNDEF);

    if (!isDefined(candidate)) continue;
    double improvement = calculateImprovement(candidate.first);
    // std::cout << "Shifting " << m_conqueror << " - (" << candidate.first << "," << candidate.second << "): " << improvement << std::endl;
    //if (improvement < 0) { char c = getchar(); if (c == 'E') return false; }
    if (improvement < 0 && !isNodeCrucial(m_state, candidate.first, candidate.second, m_conqueror))
    {
      // std::cout << "Found valid shifting!" << std::endl;
      m_bestImprovement = improvement;
      m_valid = true;
      m_bestContested = candidate.second;
      m_victim = candidate.first;
      break;
    }
  }
  return m_valid;
}

double MutationFrontierShifting::calculateImprovement(fuint32_t victim)
{ // todo: generic on EmbeddingBase
  int victimLength = static_cast<int>(m_state.getSuperVertexSize(victim));
  int conquerorLength = static_cast<int>(m_state.getSuperVertexSize(m_conqueror));
  if (victimLength == 0) return MAXFLOAT;

  return pow(victimLength - 1, 2) + pow(conquerorLength + 1, 2)
        - pow(victimLength, 2) - pow(conquerorLength, 2);
}

void MutationFrontierShifting::execute()
{
  // std::cout << "Conqueror=" << m_conqueror << "; Contested=" << m_bestContested << "; Victim=" << m_victim << std::endl;
  // getchar();
  m_manager.deleteMappingPair(m_victim, m_bestContested);
  if (!containsPair(m_manager.getMapping(), m_conqueror, m_bestContested)) m_manager.insertMappingPair(m_conqueror, m_bestContested);
  m_manager.commit();

  if (m_state.hasVisualizer())
  {
    m_manager.getVisualizer()->draw(m_manager.getMapping(), [this](std::ostream& svg)
    {
      svg << "FrontierShifting. Conqueror "
          << this->getConqueror() << ", victim "
          << this->getVictim() << ", contested "
          << this->getContested();
    });
  }
}
