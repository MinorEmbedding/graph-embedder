#ifndef __MAJORMINER_DEBUG_UTILS_HPP_
#define __MAJORMINER_DEBUG_UTILS_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  void printAdjacencyList(const adjacency_list_t& adj);

  void printNodeset(const nodeset_t& nodeset);

  void printVertexNumberMap(const VertexNumberMap& m);
}

#endif