#include "embedding_manager.hpp"
#include "majorminer.hpp"


using namespace majorminer;

#define ADJUST(...)         \
    __VA_ARGS__;            \
    m_suite.__VA_ARGS__


void EmbeddingManager::mapNode(fuint32_t node, fuint32_t targetNode)
{
  if (!m_changesToPropagate.empty()) synchronize();
  m_lastNode = node;
  DEBUG(std::cout << node << " -> " << targetNode << std::endl;)
  ADJUST(m_nodesOccupied.insert(targetNode));
  ADJUST(m_mapping.insert(std::make_pair(node, targetNode)));
  ADJUST(m_reverseMapping.insert(std::make_pair(targetNode, node)));
  ADJUST(m_targetNodesRemaining.unsafe_extract(targetNode));
}

void EmbeddingManager::mapNode(fuint32_t node, const nodeset_t& targetNodes)
{
  if (!m_changesToPropagate.empty()) synchronize();
  m_lastNode = node;
  DEBUG(OUT_S << node << " -> {";)
  for(auto targetNode : targetNodes)
  {
    DEBUG(OUT_S << " " << targetNode;)
    ADJUST(m_nodesOccupied.insert(targetNode));
    ADJUST(m_mapping.insert(std::make_pair(node, targetNode)));
    ADJUST(m_reverseMapping.insert(std::make_pair(targetNode, node)));
    ADJUST(m_targetNodesRemaining.unsafe_extract(targetNode));

  }
  DEBUG(OUT_S << " }" << std::endl;)
}
#undef ADJUST


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

int EmbeddingManager::numberFreeNeighborsNeeded(fuint32_t sourceNode)
{
  return 2 * m_suite.m_sourceNeededNeighbors[sourceNode].load()
    - std::max(m_sourceFreeNeighbors[sourceNode].load(), 0);
}

void EmbeddingManager::commit()
{
  m_changesToPropagate.push(EmbeddingChange{});
}

void EmbeddingManager::synchronize()
{
  EmbeddingChange change{};
  while(!m_changesToPropagate.empty() && m_nbCommitsRemaining > 0)
  {
    bool success = m_changesToPropagate.try_pop(change);
    if (!success) continue;
    switch(change.m_type)
    {
      case ChangeType::DEL_MAPPING:
      {
        eraseSinglePair(m_suite.m_mapping, change.m_a, change.m_b);
        eraseSinglePair(m_suite.m_reverseMapping, change.m_b, change.m_a);
        m_changeHistory[change.m_a].m_timestampNodeChanged = m_time.load();
        break;
      }
      case ChangeType::INS_MAPPING:
      {
        m_suite.m_mapping.insert(std::make_pair(change.m_a, change.m_b));
        m_suite.m_reverseMapping.insert(std::make_pair(change.m_b, change.m_a));
        m_changeHistory[change.m_a].m_timestampNodeChanged = m_time.load();
        break;
      }
      case ChangeType::FREE_NEIGHBORS:
      {
        m_suite.m_sourceFreeNeighbors[change.m_a] = change.m_b;
        m_changeHistory[change.m_a].m_timestampEdgeChanged = m_time.load();
        break;
      }
      case ChangeType::OCCUPY_NODE:
      {
        m_suite.m_targetNodesRemaining.unsafe_erase(change.m_a);
        m_suite.m_nodesOccupied.unsafe_erase(change.m_a);
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
  m_lastNode = (fuint32_t)-1;
}
