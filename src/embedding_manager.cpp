#include "embedding_manager.hpp"
#include "majorminer.hpp"


using namespace majorminer;


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
    m_nbCommitsRemaining(0), m_time(1)
{
  const auto& targetNodes = m_state.getRemainingTargetNodes();
  m_targetNodesRemaining.insert(targetNodes.begin(), targetNodes.end());
}


void EmbeddingManager::setFreeNeighbors(fuint32_t node, fuint32_t nbNeighbors)
{
  std::cout << "Start free" << std::endl;
  m_changesToPropagate.push(EmbeddingChange{ChangeType::FREE_NEIGHBORS, node, nbNeighbors});
  m_sourceFreeNeighbors[node] = nbNeighbors;
  std::cout << "End free" << std::endl;
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

int EmbeddingManager::numberFreeNeighborsNeeded(fuint32_t sourceNode)
{ // TODO: rework and correct?!
  return 2 * m_state.getSourceNeededNeighbors()[sourceNode].load()
    - std::max(m_sourceFreeNeighbors[sourceNode].load(), 0);
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
