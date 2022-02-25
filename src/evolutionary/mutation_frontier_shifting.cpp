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
  return m_valid && isDefined(m_bestContested) && calculateImprovement() < 0
        && isNodeCrucial(m_manager, m_victim, m_bestContested);
}

bool MutationFrontierShifting::prepare()
{
  // find a victim node for which conqueror is connected to and
  // all other node is not crucial for victim
  std::cout << "Preparing shifting. " << m_conqueror << std::endl;
  m_valid = false;
  ShiftingCandidates cands = m_manager.getCandidatesFor(m_conqueror);
  if (!isCandidateValid(cands))
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
    double improvement = calculateImprovement();
    std::cout << "Shifting " << m_conqueror << ": " << improvement << std::endl;
    if (improvement < 0 && !isNodeCrucial(m_state, candidate.first, candidate.second))
    {
      std::cout << "Found valid shifting!" << std::endl;
      m_bestImprovement = improvement;
      m_valid = true;
      m_bestContested = candidate.second;
      m_victim = candidate.first;
      break;
    }
  }
  return m_valid;
}

double MutationFrontierShifting::calculateImprovement()
{ // todo: generic on EmbeddingBase
  fuint32_t victimLength = m_state.getSuperVertexSize(m_victim);
  fuint32_t conquerorLength = m_state.getSuperVertexSize(m_conqueror);
  if (victimLength == 0) return MAXFLOAT;

  return pow(victimLength - 1, 2) + pow(conquerorLength + 1, 2)
        - pow(victimLength, 2) - pow(conquerorLength, 2);
}

void MutationFrontierShifting::execute()
{
  m_manager.insertMappingPair(m_conqueror, m_bestContested);
  m_manager.deleteMappingPair(m_victim, m_bestContested);

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
