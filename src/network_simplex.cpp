#include "network_simplex.hpp"

using namespace majorminer;

#define PREVENT_TAKING 100
#define OCCUPIED 10
#define FREE 1

void NetworkSimplexWrapper::embeddNode(fuint32_t node)
{
  auto adjacentIt = m_suite->m_source.equal_range(node);
  clear();
  capacity_t n = 0;
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    if (m_suite->m_mapping.find(adjacentNode->second) != m_suite->m_mapping.end()) n++;
  }
  if (n < 2) throw std::runtime_error("Invalid number of embedded adjacent nodes! < 2...");
  auto lowered = n - 1;
  // DEBUG(OUT_S << "Flow of " << lowered << " needed." << std::endl;)

  LemonArcMap<cost_t> costs(m_graph);
  LemonArcMap<capacity_t> caps(m_graph);

  for (const auto& arc : *(m_suite->m_targetGraph))
  {
    auto uNode = createNode(arc.first);
    auto vNode = createNode(arc.second);
    LemonArc uv = m_graph.addArc(uNode, vNode);
    LemonArc vu = m_graph.addArc(vNode, uNode);
    m_edgeMap.insert(std::make_pair(arc, std::make_pair(uv, vu)));
    caps[uv] = lowered;
    caps[vu] = lowered;
    costs[uv] = determineCost(arc.first);
    costs[vu] = determineCost(arc.second);
  }

  LemonNode s = m_graph.addNode();
  LemonNode t = m_graph.addNode();
  // define nodes for construction
  bool first = true;
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    auto embeddingPath = m_suite->m_mapping.equal_range(adjacentNode->second);
    if (embeddingPath.first == embeddingPath.second) continue;
    if (first)
    { // define s node connection
      auto toNode = m_nodeMap[embeddingPath.first->second];
      auto arc = m_graph.addArc(s, toNode);
      costs[arc] = 0;
      caps[arc] = lowered;
      first = false;
      adjustCosts(embeddingPath.first->second, costs);
      continue;
    }
    // DEBUG(OUT_S << "Adding construction for node " << adjacentNode->second << std::endl;)
    LemonNode constructionNode = m_graph.addNode();
    auto out = m_graph.addArc(constructionNode, t);
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



  NetworkSimplex ns(m_graph);
  ns.costMap(costs).upperMap(caps).stSupply(s, t, lowered);
  LemonArcMap<capacity_t> flows(m_graph);
  NetworkSimplex::ProblemType status = ns.run();
  if (status == NetworkSimplex::OPTIMAL)
  {
    LemonArcMap<capacity_t> flows{m_graph};
    ns.flowMap(flows);
    for (const auto& arc : m_edgeMap)
    {
      // DEBUG(OUT_S << "(" << arc.first.first << "," << arc.first.second << "): " << flows[arc.second.first] << std::endl;)
      // DEBUG(OUT_S << "(" << arc.first.second << "," << arc.first.first << "): " << flows[arc.second.second] << std::endl;)
      if (flows[arc.second.first] > 0) m_mapped.insert(arc.first.first);
      if (flows[arc.second.second] > 0) m_mapped.insert(arc.first.second);
    }
    m_suite->mapNode(node, m_mapped);
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
  auto adjacentIt = m_suite->m_target.equal_range(node);
  for (auto it = adjacentIt.first; it != adjacentIt.second; ++it)
  {
    const auto& edges = getArcPair(node, it->second);
    if (m_suite->m_targetNodesRemaining.contains(it->second))
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
  if (m_suite->m_nodesOccupied.contains(node)) return OCCUPIED;
  return FREE;
}

void NetworkSimplexWrapper::clear()
{
  m_graph.clear();
  m_nodeMap.clear();
  m_mapped.clear();
  m_edgeMap.clear();
}
