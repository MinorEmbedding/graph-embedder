#ifndef __MAJORMINER_CSC_PROBLEM_HPP_
#define __MAJORMINER_CSC_PROBLEM_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{
  adjacency_list_t extractSubgraph(const EmbeddingBase& base, fuint32_t sourceNode);

  bool isNodeCrucial(const EmbeddingBase& base, fuint32_t sourceNode, fuint32_t targetNode);
}


#endif