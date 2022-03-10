#include "initial/network_simplex.hpp"

#include <common/embedding_state.hpp>
#include <common/embedding_manager.hpp>


using namespace majorminer;

#define PREVENT_TAKING 100
#define OCCUPIED 10
#define FREE 1


NetworkSimplexWrapper::capacity_t NetworkSimplexWrapper::getNumberAdjacentNodes(const adjacency_list_range_iterator_t& adjacentIt) const
{
  const auto& mapping = m_state.getMapping();
  capacity_t n = 0;
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    if (mapping.contains(adjacentNode->second)) n++;
  }
  if (n < 2) throw std::runtime_error("Invalid number of embedded adjacent nodes! < 2...");
  return n;
}

void NetworkSimplexWrapper::constructLemonGraph(LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps)
{
  // Construct base graph
  const auto& targetGraph = *m_state.getTargetGraph();
  for (const auto& arc : targetGraph)
  {
    auto uNode = createNode(arc.first);
    auto vNode = createNode(arc.second);
    LemonArc uv = m_graph.addArc(uNode, vNode);
    LemonArc vu = m_graph.addArc(vNode, uNode);
    m_edgeMap.insert(std::make_pair(arc, std::make_pair(uv, vu)));
    caps[uv] = m_numberAdjacentLowered;
    caps[vu] = m_numberAdjacentLowered;
    costs[uv] = determineCost(arc.first);
    costs[vu] = determineCost(arc.second);
  }
}


void NetworkSimplexWrapper::constructHelperNodes(LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps, const adjacency_list_range_iterator_t& adjacentIt)
{
  // define nodes for construction
  bool first = true;
  const auto& mapping = m_state.getMapping();

  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    auto embeddingPath = mapping.equal_range(adjacentNode->second);
    if (embeddingPath.first == embeddingPath.second) continue;
    if (first)
    { // define s node connection
      auto toNode = m_nodeMap[embeddingPath.first->second];
      m_sConnected = embeddingPath.first->second;
      auto arc = m_graph.addArc(*m_s, toNode);
      costs[arc] = 0;
      caps[arc] = m_numberAdjacentLowered;
      first = false;
      adjustCosts(embeddingPath.first->second, costs);
      continue;
    }


    // DEBUG(OUT_S << "Adding construction for node " << adjacentNode->second << std::endl;)
    LemonNode constructionNode = m_graph.addNode();
    auto out = m_graph.addArc(constructionNode, *m_t);
    costs[out] = 0;
    caps[out] = 1;

    for (auto targetNode = embeddingPath.first; targetNode != embeddingPath.second; ++targetNode)
    {
      LemonNode& fromNode = m_nodeMap[targetNode->second];
      auto temp = m_graph.addArc(fromNode, constructionNode);
      costs[temp] = 0;
      caps[temp] = 1;
    }
  }
}

void NetworkSimplexWrapper::embeddNode(fuint32_t node)
{
  clear();
  auto adjacentIt = m_state.getSourceAdjGraph().equal_range(node);
  m_numberAdjacentLowered = getNumberAdjacentNodes(adjacentIt) - 1;
  // DEBUG(OUT_S << "Flow of " << lowered << " needed." << std::endl;)

  LemonArcMap<cost_t> costs(m_graph);
  LemonArcMap<capacity_t> caps(m_graph);
  constructLemonGraph(costs, caps);

  LemonNode s = m_graph.addNode();
  LemonNode t = m_graph.addNode();
  m_s = &s;
  m_t = &t;
  constructHelperNodes(costs, caps, adjacentIt);

  NetworkSimplex ns(m_graph);
  ns.costMap(costs).upperMap(caps).stSupply(*m_s, *m_t, m_numberAdjacentLowered);
  LemonArcMap<capacity_t> flows(m_graph);
  NetworkSimplex::ProblemType status = ns.run();
  if (status == NetworkSimplex::OPTIMAL)
  {
    LemonArcMap<capacity_t> flows{m_graph};
    ns.flowMap(flows);
    fuint32_t nbOutFlows = 0;
    for (const auto& arc : m_edgeMap)
    {
      // DEBUG(OUT_S << "(" << arc.first.first << "," << arc.first.second << "): " << flows[arc.second.first] << std::endl;)
      // DEBUG(OUT_S << "(" << arc.first.second << "," << arc.first.first << "): " << flows[arc.second.second] << std::endl;)
      if (flows[arc.second.first] > 0)
      {
        m_mapped.insert(arc.first.first);
        if (arc.first.first == m_sConnected) nbOutFlows++;
      }
      if (flows[arc.second.second] > 0)
      {
        if (arc.first.second == m_sConnected) nbOutFlows++;
        m_mapped.insert(arc.first.second);
      }
    }
    // OUT_S << "Number outflows " << nbOutFlows << std::endl;
    //if (nbOutFlows < 2 && m_mapped.size() > 1) m_mapped.unsafe_erase(m_sConnected);
    // m_embeddingManager.mapNode(node, m_mapped);
  }
  else if(status == NetworkSimplex::INFEASIBLE)
  {
    throw std::runtime_error("Infeasible in NetworkSimplex...");
  }
  else
  {
    throw std::runtime_error("Unbounded in NetworkSimplex");
  }
}


void NetworkSimplexWrapper::adjustCosts(fuint32_t node, LemonArcMap<cost_t>& costs)
{
  auto adjacentIt = m_state.getTargetAdjGraph().equal_range(node);
  const auto& targetNodesRemaining = m_state.getRemainingTargetNodes();
  for (auto it = adjacentIt.first; it != adjacentIt.second; ++it)
  {
    const auto& edges = getArcPair(node, it->second);
    if (targetNodesRemaining.contains(it->second))
    {
      costs[edges.first] = FREE;
      costs[edges.second] = FREE;
    }
    else
    {
      costs[edges.first] = PREVENT_TAKING;
      costs[edges.second] = PREVENT_TAKING;
    }
  }
}
const NetworkSimplexWrapper::LemonArcPair& NetworkSimplexWrapper::getArcPair(fuint32_t n1, fuint32_t n2)
{
  auto findIt = m_edgeMap.find(edge_t{n1, n2});
  if (findIt != m_edgeMap.end())
  {
    return findIt->second;
  }
  return m_edgeMap[edge_t{n2, n1}];
}

NetworkSimplexWrapper::LemonNode NetworkSimplexWrapper::createNode(fuint32_t node)
{
  auto findIt = m_nodeMap.find(node);
  if (findIt == m_nodeMap.end())
  {
    m_nodeMap.insert(std::make_pair(node, m_graph.addNode()));
    return m_nodeMap[node];
  }
  return findIt->second;
}

NetworkSimplexWrapper::cost_t NetworkSimplexWrapper::determineCost(fuint32_t node)
{
  return m_state.isNodeOccupied(node) ? OCCUPIED : FREE;
}

void NetworkSimplexWrapper::clear()
{
  m_graph.clear();
  m_nodeMap.clear();
  m_mapped.clear();
  m_edgeMap.clear();

  m_s = nullptr;
  m_t = nullptr;
  m_numberAdjacentLowered = 0;
  m_sConnected = -1;
}
