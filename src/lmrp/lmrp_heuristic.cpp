#include <lmrp/lmrp_heuristic.hpp>

#include <common/embedding_state.hpp>
#include <common/utils.hpp>

using namespace majorminer;


LMRPHeuristic::LMRPHeuristic(EmbeddingState& state, vertex_t target)
  : m_state(state), m_done(false), m_numberOverlaps(0), m_numberMapped(0)
{
  LMRPSubgraph* gen = state.getSubgraphGen();
  if (gen != nullptr)
  {
    bool success = gen->getSubgraph(target, m_crater);
    if (!success) m_done = true;
  }
  else m_done = true;
  optimize();
}


void LMRPHeuristic::optimize()
{
  if (m_done) return;
  calculatePreviousFitness();
  buildBorder();
  identifyEdges();
  identifyComponents();
  identifyDestroyed();

  solve();
}

void LMRPHeuristic::buildBorder()
{
  for (vertex_t target : m_crater)
  {
    m_state.iterateTargetGraphAdjacent(target,
      [&](vertex_t targetAdj){
        if (!m_crater.contains(targetAdj)) m_border.insert(targetAdj);
    });
  }
}

// identify all edges of the source graph embedded in m_crater
void LMRPHeuristic::identifyEdges()
{
  identifyEdgesFrom(m_crater);
  identifyEdgesFrom(m_border);

  convertToAdjacencyList(m_sourceAdjacencies, m_edges);
}

void LMRPHeuristic::identifyEdgesFrom(const nodeset_t& from)
{
  const auto& sourceGraph = *m_state.getSourceGraph();
  const auto& targetGraph = *m_state.getTargetGraph();
  const auto& reverse = m_state.getReverseMapping();
  for (auto itA = from.begin(); itA != from.end(); ++itA)
  {
    for (auto itB = itA; itB != m_crater.end(); ++itB)
    {
      if (itB == itA) itB++;
      else if (containsEdge(targetGraph, edge_t{*itA, *itB}))
      {
        auto rangeA = reverse.equal_range(*itA);
        auto rangeB = reverse.equal_range(*itB);
        for (auto revA = rangeA.first; revA != rangeA.second; ++revA)
        {
          for (auto revB = rangeB.first; revB != rangeB.second; ++revB)
          {
            edge_t edge{revA->second, revB->second};
            if (containsEdge(sourceGraph, edge))
            {
              m_edges.insert(orderedPair(edge));
            }
          }
        }
      }
    }
  }
}

void LMRPHeuristic::identifyComponents()
{
  const auto& targetGraph = m_state.getTargetAdjGraph();

  graph_t borderMapped{};
  graph_t subgraph{};
  buildSubgraphs(borderMapped, subgraph);


  Stack<adjacency_list_range_iterator_t> dfsStack{};
  Stack<vertex_t> currentReachable{};
  vertex_t source;
  while(!borderMapped.empty())
  {
    auto& startVertex = *borderMapped.begin();
    dfsStack.push(targetGraph.equal_range(startVertex.first));
    source = startVertex.second;
    currentReachable.push(startVertex.first);
    subgraph.unsafe_erase(startVertex);
    borderMapped.unsafe_erase(startVertex);

    // Conduct DFS
    while(!dfsStack.empty())
    {
      if (empty_range(dfsStack.top())) dfsStack.pop();
      else
      {
        vertex_t neighbor = dfsStack.top().first->second;
        dfsStack.top().first++;
        edge_t mapped{neighbor, source};
        if (!subgraph.contains(mapped)) continue;
        dfsStack.push(targetGraph.equal_range(neighbor));
        if (borderMapped.contains(mapped)) currentReachable.push(neighbor);
        subgraph.unsafe_erase(mapped);
        borderMapped.unsafe_erase(mapped);
      }
    }

    // Use the DFS results (i. e. currentReachable for source)
    m_componentsList.push_back(ConnectedList{source,
        m_componentVertices.size(), currentReachable.size()});
    while(!currentReachable.empty())
    {
      m_componentVertices.push_back(currentReachable.top());
        currentReachable.pop();
    }
  }
}

void LMRPHeuristic::buildSubgraphs(graph_t& borderMapped, graph_t& subgraph)
{
  const auto& reverse = m_state.getReverseMapping();
  for (auto borderVertex : m_border)
  {
    auto range = reverse.equal_range(borderVertex);
    for (auto it = range.first; it != range.second; ++it)
    {
      borderMapped.insert(*it);
    }
  }
  subgraph.insert(borderMapped.begin(), borderMapped.end());
  for (auto craterVertex : m_crater)
  {
    auto range = reverse.equal_range(craterVertex);
    for (auto it = range.first; it != range.second; ++it)
    {
      subgraph.insert(*it);
    }
  }
}

void LMRPHeuristic::calculatePreviousFitness()
{
  const auto& reverse = m_state.getReverseMapping();
  for (vertex_t target : m_crater)
  {
    fuint32_t count = reverse.count(target);
    if (count > 0) m_numberMapped++;
    if (count > 1) m_numberOverlaps += (count - 1);
  }
}

void LMRPHeuristic::identifyDestroyed()
{
  for (auto target : m_crater)
  {
    m_state.iterateReverseMapping(target,
      [&](vertex_t source){
        m_completelyDestroyed.insert(source);
    });
  }
  for (const auto& component : m_componentsList)
  {
    m_completelyDestroyed.unsafe_erase(component.m_source);
  }
}

void LMRPHeuristic::solve()
{
  std::sort(m_componentsList.begin(), m_componentsList.end());
  fuint32_t idx = 0;
  for (auto& component : m_componentsList)
  {
    if (component.m_nbMapped > 1)
    {
      connectComponent(component, idx);
    }
    idx++;
  }

  idx = 0;
  for (auto& component : m_componentsList)
  {
    if (!component.wasSatisfied()) connectComponent(component, idx);
    idx++;
  }

  embeddDestroyed();
}

void LMRPHeuristic::initializeDijkstraData()
{
  for (vertex_t target : m_crater)
  {
    m_bestPaths[target] = DijkstraVertex{target};
  }
}

void LMRPHeuristic::resetDijkstra()
{
  for (auto& vertex : m_bestPaths) vertex.second.reset();
  while(!m_dijkstraQueue.empty()) m_dijkstraQueue.pop();
}

void LMRPHeuristic::connectComponent(ConnectedList& component, fuint32_t componentIdx)
{
  if (component.m_nbMapped == 0) return;
  vertex_t root = m_componentVertices[component.m_idx];
  m_currentSource = component.m_source;

  nodeset_t toConnect{};
  if (component.m_nbMapped > 1)
  { // connect the component itself
    for (fuint32_t offset = 1; offset < component.m_nbMapped; ++offset)
    {
      toConnect.insert(m_componentVertices[component.m_idx + offset]);
    }
    while(!toConnect.empty())
    {
      runDijkstraToTarget(toConnect, root);
    }
  }
  if (componentIdx == m_componentsList.size() ||
    m_componentsList[componentIdx + 1].m_source != component.m_source)
  {
    // embedd all edges
    toConnect.clear();
    auto adjRange = m_sourceAdjacencies.equal_range(m_currentSource);
    for (auto adjIt = adjRange.first; adjIt != adjRange.second; ++adjIt)
    {
      if (m_mapping.contains(adjIt->second)) toConnect.insert(adjIt->second);
    }

    auto mappedRange = m_mapping.equal_range(m_currentSource);
    for (auto mapped = mappedRange.first; mapped != mappedRange.second; ++mapped)
    { // check for every within crater whether already connected to adjacent
      checkConnectedToSource(toConnect, mapped->second);
    }

    while(!toConnect.empty())
    {
      connectAdjacentComponents(toConnect);
    }
  }
}

void LMRPHeuristic::addBorderToMapping()
{
  const auto& reverse = m_state.getReverseMapping();
  for (auto borderVertex : m_border)
  {
    auto mappedRange = reverse.equal_range(borderVertex);
    for (auto revIt = mappedRange.first; revIt != mappedRange.second; ++revIt)
    {
      m_mapping.insert(reversePair(*revIt));
    }
    m_reverse.insert(mappedRange.first, mappedRange.second);
  }
}

fuint32_pair_t LMRPHeuristic::getLeastMappedNeighbor(vertex_t source)
{
  vertex_t neighbor = FUINT32_UNDEF;
  fuint32_t connectivity = FUINT32_UNDEF;
  fuint32_t numberMappedNeighbors = 0;

  auto range = m_sourceAdjacencies.equal_range(source);
  for (auto mappedIt = range.first; mappedIt != range.second; ++mappedIt)
  {
    fuint32_t count = m_mapping.count(mappedIt->second);
    if (count > 0 && count < connectivity)
    {
      numberMappedNeighbors++;
      connectivity = count;
      neighbor = mappedIt->second;
    }
  }
  return std::make_pair(neighbor, numberMappedNeighbors);
}

void LMRPHeuristic::embeddDestroyed()
{
  addBorderToMapping();

  for (vertex_t destroyed : m_completelyDestroyed)
  {
    embeddSingleDestroyed(destroyed);
  }
}

void LMRPHeuristic::mapToFreeVertex()
{
  vertex_t bestFound = FUINT32_UNDEF;
  fuint32_t count = FUINT32_UNDEF;

  for (auto target : m_crater)
  {
    fuint32_t c = m_reverse.count(target);
    if (c < count)
    {
      count = c;
      bestFound = target;
      if (c == 0) break;
    }
  }
  if (isDefined(count)) mapVertex(m_currentSource, bestFound);
  else throw std::runtime_error("Error in LMRP heuristic!");
}

void LMRPHeuristic::mapToSingleAdjacent(vertex_t neighbor)
{
  vertex_t bestFound = FUINT32_UNDEF;
  fuint32_t count = FUINT32_UNDEF;

  auto mappedRange = m_mapping.equal_range(neighbor);
  for (auto mappedIt = mappedRange.first; mappedIt != mappedRange.second; ++mappedIt)
  {
    m_state.iterateTargetGraphAdjacent(mappedIt->second,
      [&](fuint32_t adjTarget){
        if (m_crater.contains(adjTarget))
        {
          fuint32_t c = m_reverse.count(adjTarget);
          if (c < count)
          {
            count = c;
            bestFound = adjTarget;
          }
        }
    });
    if (count == 0) break;
  }
  if (isDefined(count)) mapVertex(m_currentSource, bestFound);
  else throw std::runtime_error("Error in LMRP heuristic!");
}

void LMRPHeuristic::embeddSingleDestroyed(vertex_t source)
{
  m_currentSource = source;
  auto neighbor = getLeastMappedNeighbor(source);
  if (neighbor.second == 0) mapToFreeVertex();
  else if (neighbor.second == 1)
  {
    mapToSingleAdjacent(neighbor.first);
    m_edges.unsafe_erase(orderedPair(source, neighbor.first));
  }
  else dijkstraDestroyed(source, neighbor.first);
}

void LMRPHeuristic::addAdjacentVertices(vertex_t source, nodeset_t& adjacent)
{
  auto range = m_sourceAdjacencies.equal_range(source);
  for (auto mappedIt = range.first; mappedIt != range.second; ++mappedIt)
  {
    if (m_mapping.contains(mappedIt->second))
    {
      adjacent.insert(mappedIt->second);
    }
  }
}

void LMRPHeuristic::dijkstraDestroyed(vertex_t source, vertex_t neighbor)
{
  resetDijkstra();
  addAllMapped(neighbor);
  nodeset_t adjacentMapped{};
  addAdjacentVertices(source, adjacentMapped);
  for (auto adj : adjacentMapped)
  { m_edges.unsafe_erase(orderedPair(edge_t{source, adj})); }

  adjacentMapped.unsafe_erase(neighbor);
  while(!adjacentMapped.empty())
  {
    DijkstraVertex next{};
    while(!m_dijkstraQueue.empty())
    {
      next = m_dijkstraQueue.top();
      m_dijkstraQueue.pop();
      if (m_bestPaths[next.m_target].visited()) continue;
      bool connected = checkConnectedToSource(adjacentMapped, next.m_target);
      if (connected)
      {
        addEmbeddedPath(next.m_target);
        break;
      }
      addSingleVertexNeighbors(next.m_target,
        next.m_overlapCnt, next.m_nonOverlapCnt);
    }
    if (!adjacentMapped.empty())
    {
      resetDijkstra();
      addAllMapped(m_currentSource);
    }
  }
}

void LMRPHeuristic::addSingleVertexNeighbors(vertex_t target,
  fuint32_t overlaps, fuint32_t length)
{
  const auto& targetGraph = m_state.getTargetAdjGraph();
  auto range = targetGraph.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    if (!m_crater.contains(it->second)) continue;
    if (m_bestPaths[it->second].wasVisited()) continue;
    auto& neighbor = m_bestPaths[it->second];
    if (neighbor.m_overlapCnt < overlaps) continue;
    bool contained = m_superVertices.contains(edge_t{m_currentSource, neighbor.m_target});
    bool count = m_reverse.count(neighbor.m_target);
    bool overlap = !contained && count > 0;
    bool lowered = neighbor.lowerTo(target,
      overlaps + (overlap ? 1 : 0),
      length + (contained ? 0 : 1));
    if (lowered) m_dijkstraQueue.push(neighbor);
  }
}

void LMRPHeuristic::runDijkstraToTarget(nodeset_t& targets, vertex_t root)
{
  resetDijkstra();
  vertex_t connectedTo = checkConnectedTo(targets, root);
  if (!isDefined(connectedTo))
  {
    vertex_t best = FUINT32_UNDEF;
    addSingleVertexNeighbors(root, 0, 0);
    DijkstraVertex next{};
    while(!m_dijkstraQueue.empty())
    {
      next = m_dijkstraQueue.top();
      m_dijkstraQueue.pop();
      if (m_bestPaths[next.m_target].visited()) continue;
      connectedTo = checkConnectedTo(targets, next.m_target);
      if (isDefined(connectedTo))
      {
        best = next.m_target;
        break;
      }
      addSingleVertexNeighbors(next.m_target,
        next.m_overlapCnt, next.m_nonOverlapCnt);
    }
    if (isDefined(best)) addEmbeddedPath(best);
  }
  if (isDefined(connectedTo)) targets.unsafe_erase(connectedTo);
  else throw std::runtime_error("No connection found!");
}

void LMRPHeuristic::addEmbeddedPath(vertex_t leaf)
{
  auto* vertex = &m_bestPaths[leaf];
  while(vertex != nullptr)
  {
    mapVertex(m_currentSource, vertex->m_target);
    if (isDefined(vertex->m_parent)) vertex = &m_bestPaths[vertex->m_parent];
    else vertex = nullptr;
  }
}

vertex_t LMRPHeuristic::checkConnectedTo(const nodeset_t& wantedTargets,
  vertex_t target)
{
  const auto& targetGraph = m_state.getTargetAdjGraph();
  auto adjRange = targetGraph.equal_range(target);
  for (auto adjIt = adjRange.first; adjIt != adjRange.second; ++adjIt)
  {
    if (wantedTargets.contains(adjIt->second)) return adjIt->second;
  }

  return FUINT32_UNDEF;
}


void LMRPHeuristic::addAllMapped(vertex_t source)
{
  const auto& targetGraph = m_state.getTargetAdjGraph();
  nodeset_t closure{};
  auto range = m_mapping.equal_range(source);
  for (auto it = range.first; it != range.second; ++it)
  {
    auto adjRange = targetGraph.equal_range(it->second);
    for (auto adjIt = adjRange.first; adjIt != adjRange.second; ++adjIt)
    {
      if (m_crater.contains(adjIt->second)) closure.insert(adjIt->second);
    }
  }
  const auto& originalMapping = m_state.getMapping();
  auto originalMapped = originalMapping.equal_range(source);
  for (auto it = originalMapped.first; it != originalMapped.second; ++it)
  {
    if (m_border.contains(it->second)) closure.insert(it->second);
  }
  for (auto it = range.first; it != range.second; ++it)
  {
    closure.unsafe_erase(it->second);
  }

  for (vertex_t adj : closure)
  {
    m_bestPaths[adj] = DijkstraVertex{adj};
    m_dijkstraQueue.push(m_bestPaths[adj]);
  }
}

void LMRPHeuristic::connectAdjacentComponents(nodeset_t& adjacent)
{
  resetDijkstra();
  addAllMapped(m_currentSource);
  DijkstraVertex next{};
  while(!m_dijkstraQueue.empty())
  {
    next = m_dijkstraQueue.top();
    m_dijkstraQueue.pop();
    if (m_bestPaths[next.m_target].visited()) continue;
    bool connected = checkConnectedToSource(adjacent, next.m_target);
    if (connected)
    {
      addEmbeddedPath(next.m_target);
      break;
    }
    addSingleVertexNeighbors(next.m_target,
      next.m_overlapCnt, next.m_nonOverlapCnt);
  }
}

bool LMRPHeuristic::checkConnectedToSource(nodeset_t& wantedSources, vertex_t target)
{
  const auto& targetGraph = m_state.getTargetAdjGraph();
  const auto& originalReverse = m_state.getReverseMapping();
  auto adjRange = targetGraph.equal_range(target);
  bool removed = false;
  for (auto adjIt = adjRange.first; adjIt != adjRange.second; ++adjIt)
  {
    auto revRange = (m_crater.contains(adjIt->second) ?
      m_reverse.equal_range(adjIt->second) :
      originalReverse.equal_range(adjIt->second));
    for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
    {
      auto extracted = wantedSources.unsafe_extract(revIt->second);
      removed |= !extracted.empty();
    }
  }

  return removed;
}

void LMRPHeuristic::mapVertex(vertex_t source, vertex_t target)
{
  edge_t mapped{source, target};
  if (!m_superVertices.contains(mapped))
  {
    m_superVertices.insert(mapped);
    m_mapping.insert(mapped);
    m_reverse.insert(reversePair(mapped));
  }
}
