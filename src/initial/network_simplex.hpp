#ifndef __MAJORMINER_NETWORK_SIMPLEX_HPP_
#define __MAJORMINER_NETWORK_SIMPLEX_HPP_

#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>

#include <majorminer_types.hpp>

namespace majorminer
{
  class NetworkSimplexWrapper
  {
    using cost_t = int;
    using capacity_t = int;
    using LemonGraph = lemon::SmartDigraph;
    using LemonNode = LemonGraph::Node;
    using LemonArc = LemonGraph::Arc;
    template <typename T>
    using LemonArcMap = LemonGraph::ArcMap<T>;
    using NetworkSimplex = lemon::NetworkSimplex<LemonGraph, capacity_t, cost_t>;
    typedef std::pair<LemonArc, LemonArc> LemonArcPair;

    public:
      NetworkSimplexWrapper(EmbeddingState& state, EmbeddingManager& embeddingManager);

      void embeddNode(vertex_t node);
      const nodeset_t& getMapped() const { return m_mapped; }

    private:
      void initialCreation();
      LemonNode createNode(vertex_t node);
      cost_t determineCost(vertex_t node);
      void adjustCosts(vertex_t node, LemonArcMap<cost_t>& costs);
      void setupCostsAndCaps();
      const LemonArcPair& getArcPair(vertex_t n1, vertex_t n2);
      vertex_t chooseSource(vertex_t source) const;

      void createCheapArc(LemonNode& from, LemonNode& to, LemonArcMap<cost_t>& costs,
          LemonArcMap<capacity_t>& caps, bool constructionArc = false, capacity_t capacity = 1);

      capacity_t getNumberAdjacentNodes(const adjacency_list_range_iterator_t& adjacentIt) const;
      void constructLemonGraph();
      void constructHelperNodes(LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps,
          const adjacency_list_range_iterator_t& adjacentIt);

      LemonNode& getNextRootNode();
      void clear();

    private:
      LemonGraph m_graph;
      EmbeddingState& m_state;
      EmbeddingManager& m_embeddingManager;
      UnorderedMap<vertex_t, LemonNode> m_nodeMap;
      UnorderedMap<edge_t, LemonArcPair, PairHashFunc<vertex_t>> m_edgeMap;
      UnorderedSet<vertex_t> m_mapped;

      std::unique_ptr<LemonArcMap<cost_t>> m_costMap;
      std::unique_ptr<LemonArcMap<capacity_t>> m_capMap;
      std::unique_ptr<LemonArcMap<capacity_t>> m_flowMap;

      Vector<LemonNode> m_rootVertices;
      Vector<LemonArc> m_treeConstructionArcs;

      LemonNode m_s;
      LemonNode m_t;
      capacity_t m_numberAdjacent;
      fuint32_t m_sConnected;
      fuint32_t m_rootCounter;

      bool m_initialized;
  };

}


#endif