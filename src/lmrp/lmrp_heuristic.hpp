#ifndef __MAJORMINER_LMRP_HEURISTIC_HPP_
#define __MAJORMINER_LMRP_HEURISTIC_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{
  struct ConnectedList
  {
    ConnectedList(vertex_t source, fuint32_t idx, fuint32_t nbMapped)
      : m_source(source), m_idx(idx),
        m_nbMapped(nbMapped), m_satisfied(false) {}

    bool wasSatisfied() const { return m_satisfied; }
    void satisfied() { m_satisfied = true; }

    vertex_t m_source;
    fuint32_t m_idx;
    fuint32_t m_nbMapped;
    bool m_satisfied;

    friend bool operator<(const ConnectedList& c1, const ConnectedList& c2)
    {
      return c1.m_source < c2.m_source;
    }
  };

  class LMRPHeuristic
  {
    public:
      LMRPHeuristic(EmbeddingState& state, vertex_t target);
      void optimize();

    private:
      void buildBorder();
      void identifyEdges();
      void identifyComponents();
      void buildSubgraphs(graph_t& borderMapped, graph_t& subgraph);
      void calculatePreviousFitness();
      void identifyDestroyed();
      void findSol();


    private:
      const EmbeddingState& m_state;
      bool m_done;
      fuint32_t m_numberOverlaps;
      fuint32_t m_numberMapped;

      nodeset_t m_crater;
      nodeset_t m_border;
      nodeset_t m_completelyDestroyed;
      graph_t m_edges;
      adjacency_list_t m_sourceAdjacencies;

      Vector<ConnectedList> m_componentsList;
      Vector<vertex_t> m_componentVertices;

  };


}

#endif
