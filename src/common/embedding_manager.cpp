#include "common/embedding_manager.hpp"

#include <common/utils.hpp>
#include <common/embedding_state.hpp>
#define CACHE_CAPACITY 128

using namespace majorminer;

namespace
{
  ShiftingCandidates getEmptyCandidate()
  {
    return ShiftingCandidates{0, std::shared_ptr<fuint32_t[]>() };
  }
}

void EmbeddingManager::mapNode(fuint32_t node, fuint32_t targetNode)
{
  if (!m_changesToPropagate.empty()) synchronize();
  m_lastNode = node;
  DEBUG(std::cout << node << " -> " << targetNode << std::endl;)

  m_nodesOccupied.insert(targetNode);
  m_mapping.insert(std::make_pair(node, targetNode));
  m_reverseMapping.insert(std::make_pair(targetNode, node));
  m_targetNodesRemaining.unsafe_extract(targetNode);
  m_state.mapNode(node, targetNode);
}

void EmbeddingManager::mapNode(fuint32_t node, const nodeset_t& targetNodes)
{
  if (!m_changesToPropagate.empty()) synchronize();
  m_lastNode = node;
  DEBUG(OUT_S << node << " -> {";)
  for(auto targetNode : targetNodes)
  {
    DEBUG(OUT_S << " " << targetNode;)
    m_nodesOccupied.insert(targetNode);
    m_mapping.insert(std::make_pair(node, targetNode));
    m_reverseMapping.insert(std::make_pair(targetNode, node));
    m_targetNodesRemaining.unsafe_extract(targetNode);

  }
  DEBUG(OUT_S << " }" << std::endl;)
  m_state.mapNode(node, targetNodes);
}
#undef ADJUST

EmbeddingManager::EmbeddingManager(EmbeddingSuite& suite, EmbeddingState& state)
  : m_suite(suite), m_state(state),
    m_candidateCache([](fuint32_t) {return getEmptyCandidate(); }, CACHE_CAPACITY),
    m_nbCommitsRemaining(0), m_time(1)
{
  const auto& targetNodes = m_state.getRemainingTargetNodes();
  m_targetNodesRemaining.insert(targetNodes.begin(), targetNodes.end());
}


void EmbeddingManager::setFreeNeighbors(fuint32_t node, fuint32_t nbNeighbors)
{
  m_changesToPropagate.push(EmbeddingChange{ChangeType::FREE_NEIGHBORS, node, nbNeighbors});
  m_sourceFreeNeighbors[node] = nbNeighbors;
}

void EmbeddingManager::deleteMappingPair(fuint32_t source, fuint32_t target)
{
  m_changesToPropagate.push(EmbeddingChange{ChangeType::DEL_MAPPING, source, target});
  eraseSinglePair(m_mapping, source, target);
  eraseSinglePair(m_reverseMapping, target, source);
}

void EmbeddingManager::insertMappingPair(fuint32_t source, fuint32_t target)
{
  m_changesToPropagate.push(EmbeddingChange{ChangeType::INS_MAPPING, source, target});
  m_mapping.insert(std::make_pair(source, target));
  m_reverseMapping.insert(std::make_pair(target, source));
}

void EmbeddingManager::occupyNode(fuint32_t target)
{
  m_changesToPropagate.push(EmbeddingChange{ChangeType::OCCUPY_NODE, target});
  m_nodesOccupied.insert(target);
  m_targetNodesRemaining.unsafe_erase(target);
}

int EmbeddingManager::numberFreeNeighborsNeeded(fuint32_t sourceNode) const
{ // TODO: rework and correct?!
  auto it = m_sourceFreeNeighbors.find(sourceNode);
  return 2 * m_state.getSourceNeededNeighbors()[sourceNode].load()
    - (it == m_sourceFreeNeighbors.end() ? 0 : std::max(it->second.load(), 0));
}

void EmbeddingManager::commit()
{
  m_changesToPropagate.push(EmbeddingChange{});
  m_nbCommitsRemaining++;
}

void EmbeddingManager::synchronize()
{
  EmbeddingChange change{};
  auto& mapping = m_state.getMapping();
  auto& revMapping = m_state.getReverseMapping();
  auto& sourceFreeNeighbors = m_state.getSourceFreeNeighbors();
  auto& nodesOccupied = m_state.getNodesOccupied();
  auto& targetNodesRemaining = m_state.getRemainingTargetNodes();
  while(!m_changesToPropagate.empty() && m_nbCommitsRemaining > 0)
  {
    bool success = m_changesToPropagate.try_pop(change);
    if (!success) break;
    switch(change.m_type)
    {
      case ChangeType::DEL_MAPPING:
      {
        eraseSinglePair(mapping, change.m_a, change.m_b);
        eraseSinglePair(revMapping, change.m_b, change.m_a);
        m_changeHistory[change.m_a].m_timestampNodeChanged = m_time.load();
        break;
      }
      case ChangeType::INS_MAPPING:
      {
        mapping.insert(std::make_pair(change.m_a, change.m_b));
        revMapping.insert(std::make_pair(change.m_b, change.m_a));
        m_changeHistory[change.m_a].m_timestampNodeChanged = m_time.load();
        break;
      }
      case ChangeType::FREE_NEIGHBORS:
      {
        sourceFreeNeighbors[change.m_a] = change.m_b;
        m_changeHistory[change.m_a].m_timestampEdgeChanged = m_time.load();
        break;
      }
      case ChangeType::OCCUPY_NODE:
      {
        targetNodesRemaining.unsafe_erase(change.m_a);
        nodesOccupied.unsafe_erase(change.m_a);
        m_changeHistory[change.m_a].m_timestampNodeChanged = m_time.load();
        break;
      }
      case ChangeType::COMMIT:
      {
        m_nbCommitsRemaining--;
        break;
      }
    }
  }
}

void EmbeddingManager::clear()
{
  for (auto& entry : m_changeHistory)
  {
    entry.second.clear();
  }
  m_time = 0;
  m_changesToPropagate.clear();
  m_nbCommitsRemaining = 0;
}

ShiftingCandidates EmbeddingManager::getCandidatesFor(fuint32_t conquerorNode)
{
  auto handle = m_candidateCache[conquerorNode];
  if (handle) return handle.value();
  else return getEmptyCandidate();
}

ShiftingCandidates EmbeddingManager::setCandidatesFor(fuint32_t conquerorNode, nodeset_t& candidates)
{
  ShiftingCandidates element = std::make_pair(candidates.size(), std::make_shared<fuint32_t[]>(candidates.size()));
  fuint32_t* writePtr = element.second.get();
  for (auto candidate : candidates) *writePtr++ = candidate;
  m_candidateCache[conquerorNode].value() = element;
  return element;
}


const graph_t* EmbeddingManager::getSourceGraph() const { return m_state.getSourceGraph(); }
const graph_t* EmbeddingManager::getTargetGraph() const { return m_state.getTargetGraph(); }
const adjacency_list_t& EmbeddingManager::getSourceAdjGraph() const { return m_state.getSourceAdjGraph(); }
const adjacency_list_t& EmbeddingManager::getTargetAdjGraph() const { return m_state.getTargetAdjGraph(); }
const embedding_mapping_t& EmbeddingManager::getMapping() const { return m_mapping; }
const embedding_mapping_t& EmbeddingManager::getReverseMapping() const { return m_reverseMapping; }
const nodeset_t& EmbeddingManager::getNodesOccupied() const { return m_nodesOccupied; }
const nodeset_t& EmbeddingManager::getRemainingTargetNodes() const { return m_targetNodesRemaining; }
