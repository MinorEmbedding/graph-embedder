#ifndef __MAJORMINER_CUT_VERTEX_HPP_
#define __MAJORMINER_CUT_VERTEX_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{
  // list all cutvertices in Cut
  // n is the number of distinct vertices in the subgraph
  void identifiyCutVertices(nodeset_t& cut, const adjacency_list_t& subgraph, fuint32_t n);

  // check whether node is a cut vertex (needed for validation in FrontierShifting)
  bool isCutVertex(const adjacency_list_t& subgraph, vertex_t node, fuint32_t n);

  // check whether targetNode (which is a node sourceNode is mapped to) is a cut vertex
  bool isCutVertex(const EmbeddingBase& base, vertex_t sourceNode, vertex_t targetNode);
  bool isCutVertex(const EmbeddingBase& base, nodeset_t& mappedNodes, vertex_t targetNode);

  bool areSetsConnected(const EmbeddingBase& base, const nodeset_t& setA, const nodeset_t& setB);

}


#endif