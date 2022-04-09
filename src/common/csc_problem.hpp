#ifndef __MAJORMINER_CSC_PROBLEM_HPP_
#define __MAJORMINER_CSC_PROBLEM_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{
  adjacency_list_t extractSubgraph(const EmbeddingBase& base, fuint32_t sourceNode);

  nodeset_t getEmbeddedAdjacentSourceVertices(const EmbeddingBase& base, fuint32_t sourceVertex);

  bool isNodeCrucial(const EmbeddingBase& base, fuint32_t sourceNode, fuint32_t targetNode, fuint32_t conqueror);

  bool connectsToAdjacentVertices(const EmbeddingManager& base, const nodeset_t& placement, fuint32_t sourceVertex);
}


#endif