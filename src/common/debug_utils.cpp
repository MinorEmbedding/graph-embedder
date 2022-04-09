#include <common/debug_utils.hpp>

using namespace majorminer;


void majorminer::printNodeset(const nodeset_t& nodeset)
{
  Vector<fuint32_t> nodes{};
  nodes.reserve(nodeset.size());
  nodes.assign(nodeset.begin(), nodeset.end());
  std::sort(nodes.begin(), nodes.end());

  std::cout << "{";
  for (auto node : nodes) std::cout << " " << node;
  std::cout << " }" << std::endl;
}


void majorminer::printAdjacencyList(const adjacency_list_t& adj)
{
  for (const auto& edge : adj)
  {
    std::cout << "(" << edge.first << ", " << edge.second << ")" << std::endl;
  }
}

void majorminer::printVertexNumberMap(const VertexNumberMap& m)
{
  for (const auto& p : m)
  {
    std::cout << "(" << p.first << ", " << p.second << ") ";
  }
  std::cout << std::endl;
}