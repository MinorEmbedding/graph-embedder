#include "majorminer.hpp"

using namespace majorminer;

EmbeddingSuite::EmbeddingSuite(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer)
  : m_sourceGraph(&source), m_targetGraph(&target), m_visualizer(visualizer) {
  convertToAdjacencyList(m_source, source);
  convertToAdjacencyList(m_target, target);
  for (const auto& arc : target)
  {
    m_targetNodesRemaining.insert(arc.first);
    m_targetNodesRemaining.insert(arc.second);
  }
  for (const auto& arc : *m_sourceGraph)
  {
    m_nodesRemaining[arc.first] = 0;
    m_nodesRemaining[arc.second] = 0;
    m_sourceNeededNeighbors[arc.first]++;
    m_sourceNeededNeighbors[arc.second]++;
  }
}

embedding_mapping_t EmbeddingSuite::find_embedding()
{
  while(!m_nodesRemaining.empty())
  {
    if (!m_nodesToProcess.empty())
    {
      PrioNode node{};
      auto found = m_nodesToProcess.try_pop(node);
      if (!found) continue;
      if (!m_nodesRemaining.contains(node.m_id)) continue;
      m_nodesRemaining.unsafe_erase(node.m_id);
      if (node.m_nbConnections > 1)
      {
        DEBUG(OUT_S << "Complex node to embedd: " << node.m_id << " (" << node.m_nbConnections << ")" << std::endl;)
        embeddNode(node.m_id);
        if (m_visualizer != nullptr)
        {
          m_visualizer->draw(m_mapping, [&](std::ostream& os)
            { os << "Complex adjacent node " << node.m_id << " (" << node.m_nbConnections << ")";}
          );
        }
        // prepareFrontierShifting(node.m_id, node.m_nbConnections);
      }
      else
      { // nbConnections = 1
        DEBUG(OUT_S << "Simple adjacent node to embedd: " << node.m_id << std::endl;)
        embeddSimpleNode(node.m_id);
        if (m_visualizer != nullptr)
        {
          m_visualizer->draw(m_mapping, [&](std::ostream& os)
            { os << "Simple adjacent node " << node.m_id << " (1).";}
          );
        }
      }
      updateConnections(node.m_id);
      tryMutations();
    }
    else
    {
      // just pick an arbitrary node and place it somewhere
      auto node = *m_nodesRemaining.begin();
      m_nodesRemaining.unsafe_erase(m_nodesRemaining.begin());
      DEBUG(OUT_S << "Trivial node to embedd: " << node.first << std::endl;)
      embeddTrivialNode(node.first);
      if (m_visualizer != nullptr)
      {
        m_visualizer->draw(m_mapping, [&](std::ostream& os)
            { os << "Trivial node " << node.first;}
        );
      }
      tryMutations();
    }
  }
  return m_mapping;
}


void EmbeddingSuite::embeddNode(fuint32_t node)
{
  embeddNodeNetworkSimplex(node);
}


void EmbeddingSuite::embeddNodeNetworkSimplex(fuint32_t node)
{
  if (m_nsWrapper.get() == nullptr) m_nsWrapper = std::make_unique<NetworkSimplexWrapper>(this);

  m_nsWrapper->embeddNode(node);
}


void EmbeddingSuite::mapNode(fuint32_t node, fuint32_t targetNode)
{
  DEBUG(std::cout << node << " -> " << targetNode << std::endl;)
  m_nodesOccupied.insert(targetNode);
  m_mapping.insert(std::make_pair(node, targetNode));
  m_reverseMapping.insert(std::make_pair(targetNode, node));
  m_targetNodesRemaining.unsafe_extract(targetNode);
  updateNeededNeighbors(node);
}

void EmbeddingSuite::mapNode(fuint32_t node, const nodeset_t& targetNodes)
{
  DEBUG(OUT_S << node << " -> {";)
  for(auto targetNode : targetNodes)
  {
    DEBUG(OUT_S << " " << targetNode;)
    m_nodesOccupied.insert(targetNode);
    m_mapping.insert(std::make_pair(node, targetNode));
    m_reverseMapping.insert(std::make_pair(targetNode, node));
    m_targetNodesRemaining.unsafe_extract(targetNode);
  }
  updateNeededNeighbors(node);
  DEBUG(OUT_S << " }" << std::endl;)
}

void EmbeddingSuite::updateNeededNeighbors(fuint32_t node)
{
  auto range = m_source.equal_range(node);
  fuint32_t nbNodes = 0;
  for (auto it = range.first; it != range.second; ++it)
  {
    if (!m_nodesRemaining.contains(it->second))
    {
      nbNodes++;
      m_sourceNeededNeighbors[it->second]--;
    }
  }
  m_sourceNeededNeighbors[node] -= nbNodes;
}

void EmbeddingSuite::updateConnections(fuint32_t node)
{
  auto adjacentRange = m_source.equal_range(node);
  for (auto adjacentIt = adjacentRange.first; adjacentIt != adjacentRange.second; ++adjacentIt)
  {
    auto findIt = m_nodesRemaining.find(adjacentIt->second);
    if (findIt != m_nodesRemaining.end())
    {
      findIt->second += 1; // one of its neighbors is now embedded
      m_nodesToProcess.push(PrioNode{findIt->first, findIt->second});
    }
  }
  identifyAffected(node);
}

void EmbeddingSuite::embeddSimpleNode(fuint32_t node)
{
  // find a node that is adjacent to the node "adjacentNode"
  auto adjacentIt = m_source.equal_range(node);
  fuint32_t adjacentNode = -1;
  for (auto n = adjacentIt.first; n != adjacentIt.second; ++n)
  {
    if (!m_nodesRemaining.contains(n->second))
    {
      adjacentNode = n->second;
      break;
    }
  }
  if (adjacentNode == (fuint32_t)-1) throw std::runtime_error("Could not find the adjacent node.");

  auto embeddedPathIt = m_mapping.equal_range(adjacentNode);
  fuint32_t bestNodeFound = adjacentNode;

  for (auto targetNode = embeddedPathIt.first; targetNode != embeddedPathIt.second && bestNodeFound == adjacentNode; ++targetNode)
  {
    // find nodes that are adjacent to targetNode (in the targetGraph)
    auto targetGraphAdjacentIt = m_target.equal_range(targetNode->second);
    for (auto targetAdjacent = targetGraphAdjacentIt.first; targetAdjacent != targetGraphAdjacentIt.second; ++targetAdjacent)
    {
      if (!m_nodesOccupied.contains(targetAdjacent->second))
      {
        bestNodeFound = targetAdjacent->second;
        break;
      }
    }
  }
  // map "node" to "bestNodeFound"
  mapNode(node, bestNodeFound);
}

void EmbeddingSuite::embeddTrivialNode(fuint32_t node)
{
  if (!m_targetNodesRemaining.empty())
  {
    auto targetNode = *m_targetNodesRemaining.begin();
    m_targetNodesRemaining.unsafe_erase(m_targetNodesRemaining.begin());
    mapNode(node, targetNode);
    updateConnections(node);
  }
  else
  {
    throw std::runtime_error("Oooops...");
  }
}

bool EmbeddingSuite::isValid() const
{
  EmbeddingValidator validator{m_mapping, *m_sourceGraph, m_target};
  return validator.isValid();
}


bool EmbeddingSuite::connectsNodes() const
{
  EmbeddingValidator validator{m_mapping, *m_sourceGraph, m_target};
  return validator.nodesConnected();
}

void EmbeddingSuite::identifyAffected(fuint32_t node)
{
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
}

int EmbeddingSuite::numberFreeNeighborsNeeded(fuint32_t sourceNode)
{
  // std::cout << "Source node " << sourceNode << " needs " << m_sourceNeededNeighbors[sourceNode].load() << " neighbors and has " << m_sourceFreeNeighbors[sourceNode].load() << std::endl;
  return 2 * m_sourceNeededNeighbors[sourceNode].load()
    - std::max(m_sourceFreeNeighbors[sourceNode].load(), 0);
}

void EmbeddingSuite::prepareFrontierShifting(fuint32_t victimNode, fuint32_t nbConnectedTo)
{
  m_cutVertices.clear();
  m_victimSubgraph.clear();
  m_victimConnections.clear();
  m_reverseConnections.clear();

  auto mapped = m_mapping.equal_range(victimNode);
  auto nbMapped = std::distance(mapped.first, mapped.second);
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
      edge_t vu{mapIt->second, innerIt->second};
      if (m_targetGraph->contains(uv) ||
        m_targetGraph->contains(vu))
      {
        m_victimSubgraph.insert(uv);
        m_victimSubgraph.insert(vu);
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
          m_victimConnections.insert(edge_t{ revIt->second, mapIt->second });
          m_reverseConnections.insert(edge_t{ mapIt->second, revIt->second });
        }
      }
    }
  }
  // frontiershifting is not allowed on cut vertices as removing them would
  // disconnect the chain. Therefore, identify cut vertices beforehand
  majorminer::identifiyCutVertices(m_cutVertices, m_victimSubgraph, nbMapped);
  m_victimSourcNode = victimNode;
}

void EmbeddingSuite::tryMutations()
{
  auto& queue = m_taskQueue;
  if (m_victimSubgraph.size() > 0)
  {
    const auto& nodesRemaining = m_nodesRemaining;
    auto victimAdjRange = m_source.equal_range(m_victimSourcNode);
    tbb::parallel_for_each(victimAdjRange.first, victimAdjRange.second,
      [&](const edge_t& edge){
        if (nodesRemaining.contains(edge.second))
        {
          queue.push(std::make_unique<MuationFrontierShifting>(this, edge.second, edge.first));
        }
    });
    m_sourceNodesAffected.unsafe_erase(m_victimSourcNode);
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
  m_sourceNodesAffected.clear();
  m_victimSubgraph.clear();
  m_cutVertices.clear();
  m_victimSourcNode = -1;
}
