#include "embedding_manager.hpp"

using namespace majorminer;

#define ADJUST(...)         \
    __VA_ARGS__;            \
    m_suite.__VA_ARGS__


void EmbeddingManager::mapNode(fuint32_t node, fuint32_t targetNode)
{
  if (!m_changesToPropagate.empty()) synchronize();
  DEBUG(std::cout << node << " -> " << targetNode << std::endl;)
  ADJUST(m_nodesOccupied.insert(targetNode));
  ADJUST(m_mapping.insert(std::make_pair(node, targetNode)));
  ADJUST(m_reverseMapping.insert(std::make_pair(targetNode, node)));
  ADJUST(m_targetNodesRemaining.unsafe_extract(targetNode));
}

void EmbeddingManager::mapNode(fuint32_t node, const nodeset_t& targetNodes)
{
  if (!m_changesToPropagate.empty()) synchronize();
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

void EmbeddingManager::commit()
{
  m_changesToPropagate.push(EmbeddingChange{});
}

void EmbeddingManager::synchronize()
{

}
