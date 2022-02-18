#ifndef __MAJORMINER_CUT_VERTEX_HPP_
#define __MAJORMINER_CUT_VERTEX_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{
  // list all cutvertices in Cut
  // n is the number of distinct vertices in the subgraph
  void identifiyCutVertices(nodeset_t& cut, const adjacency_list_t& subgraph, fuint32_t n);

  // check whether node is a cut vertex (needed for validation in FrontierShifting)
  bool isCutVertex(const adjacency_list_t& subgraph, fuint32_t node, fuint32_t n);


}


#endif