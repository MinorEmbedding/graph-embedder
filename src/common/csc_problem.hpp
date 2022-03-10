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
      const nodeset_t& getSuperVertex() const { return m_superVertex; }
      void initialize(const nodeset_t& currentMapping);
      const nodeset_t& getBetterPlacement(const nodeset_t& previous) const;

    private:
      bool isBadNode(fuint32_t target) const;
      void addNode(fuint32_t target);
      void removeNode(fuint32_t target);
      fuint32_t checkScore(const nodeset_t& placement) const;

      // checks whether a new target node would be connected to the super vertex
      bool isConnected(fuint32_t target) const;

    private:
      const EmbeddingBase& m_embedding;
      nodeset_t m_superVertex; // current super vertex
      nodeset_t m_potentialNodes;
      adjacency_list_t m_adjacencies; // (targetVertex, adjacentSource)
      fuint32_t m_sourceVertex;

      std::unique_ptr<fuint32_t[]> m_verticesList;

      RandomGen* m_rand;

      // for each neighbor of m_sourceVertex, map contains the number of
      //  vertices of m_sourceVertex connecting the two
      UnorderedMap<fuint32_t, fuint32_t> m_sourceConnections; 
      bool m_done;
  };
}


#endif