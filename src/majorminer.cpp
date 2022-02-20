#include "majorminer.hpp"

#include "common/graph_gen.hpp"
#include "common/utils.hpp"
#include "common/embedding_validator.hpp"
#include "common/embedding_visualizer.hpp"
#include "common/cut_vertex.hpp"

#include "evolutionary/mutation_extend.hpp"
#include "evolutionary/frontier_shifting_data.hpp"
#include "evolutionary/mutation_frontier_shifting.hpp"

using namespace majorminer;

EmbeddingSuite::EmbeddingSuite(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer)
  : m_state(source, target, visualizer), m_visualizer(visualizer),
    m_embeddingManager(*this, m_state), m_mutationManager(m_state, m_embeddingManager),
    m_placer(m_state, m_embeddingManager)
{ }

embedding_mapping_t EmbeddingSuite::find_embedding()
{
  const auto& nodesRemaining = m_state.getRemainingNodes();
  while(!nodesRemaining.empty())
  {
    m_placer();
    tryMutations();
  }
  return m_state.getMapping();
}


bool EmbeddingSuite::isValid() const
{
  EmbeddingValidator validator{m_state};
  return validator.isValid();
}


bool EmbeddingSuite::connectsNodes() const
{
  EmbeddingValidator validator{m_state};
  return validator.nodesConnected();
}

void EmbeddingSuite::identifyAffected(fuint32_t /* node */)
{
  /*
  m_sourceNodesAffected.clear();

  // "node" (from source graph) is now mapped to at least one
  // target node. Iterate over those, to iterate over their neighbors
  // that might be mapped to other source nodes
  const auto& reverseMapping = m_reverseMapping;
  auto& sourceNodesAffected = m_sourceNodesAffected;
  auto& sourceFreeNeighbors = m_sourceFreeNeighbors;
  const auto& targetNodesRemaining = m_targetNodesRemaining;
  auto& target = m_target;
  m_sourceFreeNeighbors[node] = 0;
  sourceNodesAffected.insert(node);
  auto mappedRange = m_mapping.equal_range(node);
  nodeset_t freeAdjNodes {};
  tbb::parallel_for_each(mappedRange.first, mappedRange.second,
    [&, this] (const fuint32_pair_t& p){
      auto adjacentRange = target.equal_range(p.second);
      nodeset_t nodes{}; // TODO: awful!
      for (auto targetAdjacent = adjacentRange.first; targetAdjacent !=  adjacentRange.second; ++targetAdjacent)
      { // if that node is free, note that, else decrement from all mapped
        if (targetNodesRemaining.contains(targetAdjacent->second)) freeAdjNodes.insert(targetAdjacent->second);
        else
        {
          auto revMapRange = reverseMapping.equal_range(targetAdjacent->second);
          for (auto revIt = revMapRange.first; revIt != revMapRange.second; ++revIt)
          {
            if (!nodes.contains(revIt->second))
            {
              sourceNodesAffected.insert(revIt->second);
              sourceFreeNeighbors[revIt->second]--;
              nodes.insert(revIt->second);
            }
          }
        }
      }
  });
  m_sourceFreeNeighbors[node] = freeAdjNodes.size();
  */
}



void EmbeddingSuite::prepareFrontierShifting(fuint32_t /* victimNode */, fuint32_t /* nbConnectedTo */)
{
  /*
  m_frontierData.clear();

  auto mapped = m_mapping.equal_range(victimNode);
  auto nbMapped = m_mapping.count(victimNode);
  if (nbMapped < 4) return; // Bascially useless (or not possible)

  nodeset_t connected{};
  auto adjRange = m_source.equal_range(victimNode);
  for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
  { if (!m_nodesRemaining.contains(adj->second)) connected.insert(adj->second); }

  for (auto mapIt = mapped.first; mapIt != mapped.second; ++mapIt)
  {
    auto innerIt = mapIt;
    ++innerIt;
    for (; innerIt != mapped.second; ++innerIt)
    {
      edge_t uv{mapIt->second, innerIt->second};
      edge_t vu{innerIt->second, mapIt->second};
      if (m_targetGraph->contains(uv) ||
        m_targetGraph->contains(vu))
      {
        m_frontierData.m_victimSubgraph.insert(uv);
        m_frontierData.m_victimSubgraph.insert(vu);
      }
    }
    auto adj = m_target.equal_range(mapIt->second);
    for (auto adjNode = adj.first; adjNode != adj.second; ++adjNode)
    {
      auto revRange = m_reverseMapping.equal_range(adjNode->second);
      for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
      {
        if (connected.contains(revIt->second))
        {
          m_frontierData.addConnection(revIt->second, mapIt->second);
        }
      }
    }
  }
  m_frontierData.setNbNodes(nbMapped);
  m_frontierData.setVictimSource(victimNode);
  m_frontierData.findCutVertices();
  */
}

void EmbeddingSuite::tryMutations()
{
  m_embeddingManager.clear();
  m_mutationManager.mutate();
  /*
  auto& queue = m_taskQueue;
  if (m_frontierData.m_victimSubgraph.size() > 0)
  {
    const auto& nodesRemaining = m_nodesRemaining;
    auto victimAdjRange = m_source.equal_range(m_frontierData.m_victimSourceNode);
    tbb::parallel_for_each(victimAdjRange.first, victimAdjRange.second,
      [&](const edge_t& edge){
        if (!nodesRemaining.contains(edge.second))
        {
          queue.push(std::make_unique<MuationFrontierShifting>(this, edge.second, edge.first));
        }
    });
    m_sourceNodesAffected.unsafe_erase(m_frontierData.m_victimSourceNode);
  }
  tbb::parallel_for_each(m_sourceNodesAffected.begin(), m_sourceNodesAffected.end(),
    [&, this](fuint32_t node){
       queue.push(std::make_unique<MutationExtend>(this, node));
  });

  while(!m_taskQueue.empty())
  {
    std::unique_ptr<GenericMutation> task;
    bool worked = m_taskQueue.try_pop(task);
    if (!worked) break;
    task->execute();
  }
  m_frontierData.clear();
  */
}
