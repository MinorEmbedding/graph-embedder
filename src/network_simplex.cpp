#include "network_simplex.hpp"

using namespace majorminer;

#define OCCUPIED 10
#define FREE 1

void NetworkSimplexWrapper::embeddNode(fuint32_t node)
{
  auto adjacentIt = m_suite->m_source.equal_range(node);
  if (adjacentIt.first == adjacentIt.second)
  { // just map to some unused node

  }
  clear();
  capacity_t n = 0;
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    if (m_suite->m_mapping.find(adjacentNode->second) != m_suite->m_mapping.end()) n++;
  }
  if (n == 1)
  { // just map to an adjacent node

  }
  auto lowered = n - 1;

  LemonArcMap<cost_t> costs(m_graph);
  LemonArcMap<capacity_t> caps(m_graph);
  LemonNode t = m_graph.addNode();

  for (const auto& arc : *(m_suite->m_sourceGraph))
  {
    auto uNode = createNode(arc.first);
    auto vNode = createNode(arc.second);
    LemonArc uv = m_graph.addArc(uNode, vNode);
    LemonArc vu = m_graph.addArc(vNode, uNode);
    m_edgeMap.insert(std::make_pair(arc, std::make_pair(uv, vu)));
    caps[uv] = lowered;
    costs[uv] = determineCost(arc.first);
    costs[vu] = determineCost(arc.second);
  }

  LemonNode s;
  // define nodes for construction
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    auto embeddingPath = m_suite->m_mapping.equal_range(adjacentNode->second);
    if (embeddingPath.first == embeddingPath.second) continue;
    if (adjacentNode == adjacentIt.first)
    { // define s node
      s = m_graph.addNode();
      auto arc = m_graph.addArc(s, m_nodeMap[embeddingPath.first->second]);
      costs[arc] = 0;
      caps[arc] = lowered;
    }

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
      if (flows[arc.second.first] > 0) m_mapped.insert(arc.first.first);
      if (flows[arc.second.second] > 0) m_mapped.insert(arc.first.second);
    }
    m_suite->mapNode(node, m_mapped);
  }
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
