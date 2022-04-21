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

  findSol();
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
  const auto& sourceGraph = *m_state.getSourceGraph();
  const auto& targetGraph = *m_state.getTargetGraph();
  const auto& reverse = m_state.getReverseMapping();
  for (auto itA = m_crater.begin(); itA != m_crater.end(); ++itA)
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

  convertToAdjacencyList(m_sourceAdjacencies, m_edges);
}

void LMRPHeuristic::identifyComponents()
{
  const auto& targetGraph = m_state.getTargetAdjGraph();
  const auto& reverse = m_state.getReverseMapping();

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

void LMRPHeuristic::findSol()
{
  std::sort(m_componentsList.begin(), m_componentsList.end());

  
}
