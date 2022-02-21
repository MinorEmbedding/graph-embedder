#include "common/csc_problem.hpp"

#include <common/cut_vertex.hpp>
#include <common/embedding_base.hpp>

using namespace majorminer;

adjacency_list_t majorminer::extractSubgraph(const EmbeddingBase& base, fuint32_t sourceNode)
{
  adjacency_list_t subgraph{};
  const auto& targetGraph = *base.getTargetGraph();
  base.iterateSourceMappingPair(sourceNode,
    [&subgraph, &targetGraph](fuint32_t targetNodeA, fuint32_t targetNodeB){
      edge_t uv{targetNodeA, targetNodeB};
      edge_t vu{targetNodeB, targetNodeA};
      if (targetGraph.contains(uv) || targetGraph.contains(vu))
      {
        subgraph.insert(uv);
        subgraph.insert(vu);
      }
  });
  return subgraph;
}


bool majorminer::isNodeCrucial(const adjacency_list_t& /* subgraph */, const EmbeddingBase& /* base */, fuint32_t /* targetNode */)
{
  return true;
}
