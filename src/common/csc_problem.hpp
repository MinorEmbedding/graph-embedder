#ifndef __MAJORMINER_CSC_PROBLEM_HPP_
#define __MAJORMINER_CSC_PROBLEM_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{
  adjacency_list_t extractSubgraph(const EmbeddingBase& base, fuint32_t sourceNode);

  bool isNodeCrucial(const EmbeddingBase& base, fuint32_t sourceNode, fuint32_t targetNode);

  class SuperVertexReducer
  {
    public:
      SuperVertexReducer(const EmbeddingBase& base, fuint32_t sourceVertex);

      void optimize();

    private:
      inline void initialize(const EmbeddingBase& base);

    private:
      nodeset_t m_superVertex; // current super vertex
      nodeset_t m_potentialNodes;
      adjacency_list_t m_adjacencies; // (targetVertex, adjacentSource)
      fuint32_t m_sourceVertex;
      bool m_done;
  };
}


#endif