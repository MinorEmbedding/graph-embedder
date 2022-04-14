
#include "cut_vertex.hpp"

#include <common/utils.hpp>
#include <common/embedding_base.hpp>

using namespace majorminer;

typedef adjacency_list_t::const_iterator adj_list_const_it;
typedef std::pair<adj_list_const_it, adj_list_const_it> equal_range_t;

struct VertexData
{
  VertexData()
    : m_parent((vertex_t)-1), m_depth(m_parent),
      m_lowest(m_parent), m_recursed(false) {}
  VertexData(vertex_t parent, fuint32_t depth)
    : m_parent(parent), m_depth(depth),
      m_lowest(depth), m_recursed(false) {}

  vertex_t m_parent;
  fuint32_t m_depth;
  fuint32_t m_lowest;
  bool m_recursed;
};

void majorminer::identifiyCutVertices(nodeset_t& cut, const adjacency_list_t& subgraph, fuint32_t n)
{
  if (subgraph.empty()) return;
  Stack<equal_range_t> nodeStack{};

  UnorderedMap<vertex_t, VertexData> properties{};

  vertex_t rootNode = subgraph.begin()->first;
  nodeStack.push(subgraph.equal_range(rootNode));
  properties.insert(std::make_pair(rootNode, VertexData{ (vertex_t)-1, 0 }));
  fuint32_t depth = 1;
  fuint32_t rootChildren = 0;

  while(!nodeStack.empty())
  {
    auto& top = nodeStack.top();
    if (top.first != top.second)
    {
      vertex_t currentNode = top.first->first;
      vertex_t nextNode = top.first->second;
      auto& p = properties[currentNode];
      if (p.m_recursed)
      { // take care of last "recursion" (not actually recursing)
        p.m_recursed = false;
        const auto& child = properties[nextNode];
        setMin(p.m_lowest, child.m_lowest);
        top.first++;
        if (currentNode != rootNode && child.m_lowest >= p.m_depth)
        { cut.insert(currentNode); }
        continue;
      }

      if (!properties.contains(nextNode))
      { // visit next node
        if (top.first->first == rootNode) rootChildren++;
        p.m_recursed = true;
        properties.insert(std::make_pair(nextNode, VertexData{currentNode, depth++}));
        nodeStack.push(subgraph.equal_range(nextNode));
      }
      else if (nextNode != p.m_parent)
      { // other node was already visited
        // lower discovery value to depth of next node
        setMin(p.m_lowest, properties[nextNode].m_depth);
        top.first++;
      }
      else top.first++;
    }
    else nodeStack.pop();
  }
  if (properties.size() != n) throw std::runtime_error("Disconnected subgraph in articulation point detection!");
  if (rootChildren > 1) cut.insert(rootNode);
}


// Conduct a depth-first search that does not visit the vertex "node".
// If all nodes were visited, then node is not a cut vertex
bool majorminer::isCutVertex(const adjacency_list_t& subgraph, vertex_t node, fuint32_t n)
{
  auto nodeFind = subgraph.find(node);
  if (nodeFind == subgraph.end()) return true;
  vertex_t rootNode = nodeFind->second;
  if (rootNode == node) throw std::runtime_error("Loop inside subgraph!");

  Stack<equal_range_t> nodeStack{};
  nodeset_t visited{};
  visited.insert(node);
  nodeStack.push(subgraph.equal_range(rootNode));
  visited.insert(rootNode);

  while(!nodeStack.empty())
  {
    auto& top = nodeStack.top();
    if (top.first != top.second)
    {
      vertex_t nextNode = top.first->second;
      top.first++;
      if (!visited.contains(nextNode))
      {
        visited.insert(nextNode);
        nodeStack.push(subgraph.equal_range(nextNode));
      }
    }
    else nodeStack.pop();
  }
  return visited.size() < n;
}



bool majorminer::isCutVertex(const EmbeddingBase& base, vertex_t sourceNode, vertex_t targetNode)
{
  nodeset_t mapped {};
  insertMappedTargetNodes(base, mapped, sourceNode);
  return isCutVertex(base, mapped, targetNode);
}

bool majorminer::isCutVertex(const EmbeddingBase& base, nodeset_t& mappedNodes, vertex_t targetNode)
{
  if (mappedNodes.size() <= 1) return true;
  mappedNodes.unsafe_erase(targetNode);
  const auto& targetAdj = base.getTargetAdjGraph();
  vertex_t adjacentTarget = FUINT32_UNDEF; // cannot use targetNode here
  auto range = targetAdj.equal_range(targetNode);
  for (auto it = range.first; it != range.second; ++it)
  {
    if (mappedNodes.contains(it->second))
    {
      adjacentTarget = it->second;
      break;
    }
  }
  if (!isDefined(adjacentTarget)) return true;
  mappedNodes.unsafe_erase(adjacentTarget);

  Stack<equal_range_t> nodeStack{};
  nodeStack.push(targetAdj.equal_range(adjacentTarget));
  while(!nodeStack.empty())
  {
    auto& top = nodeStack.top();
    if (top.first == top.second) nodeStack.pop();
    else if (top.first->second == targetNode) top.first++;
    else
    {
      vertex_t next = top.first->second;
      top.first++;
      auto val = mappedNodes.unsafe_extract(next);
      if (!val.empty())
      {
        if (mappedNodes.empty()) return false;
        nodeStack.push(targetAdj.equal_range(next));
      }
    }
  }
  return !mappedNodes.empty();
}


bool majorminer::areSetsConnected(const EmbeddingBase& base, const nodeset_t& setA, const nodeset_t& setB)
{
  bool connected = false;
  for (auto target : setA)
  {
    base.iterateTargetGraphAdjacentBreak(target, [&](vertex_t adjTarget){
      if (setB.contains(adjTarget)) connected = true;
      return connected;
    });
    if (connected) return true;
  }
  return false;
}