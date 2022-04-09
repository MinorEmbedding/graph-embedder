#ifndef __MAJORMINER_NETWORK_SIMPLEX_HPP_
#define __MAJORMINER_NETWORK_SIMPLEX_HPP_

#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>

#include "majorminer_types.hpp"

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
      NetworkSimplexWrapper(EmbeddingState& state, EmbeddingManager& embeddingManager)
        : m_state(state), m_embeddingManager(embeddingManager) { }

      void embeddNode(fuint32_t node);
      const nodeset_t& getMapped() const { return m_mapped; }

    private:
      LemonNode createNode(fuint32_t node);
      cost_t determineCost(fuint32_t node);
      void adjustCosts(fuint32_t node, LemonArcMap<cost_t>& costs);
      void clear();
      const LemonArcPair& getArcPair(fuint32_t n1, fuint32_t n2);

      capacity_t getNumberAdjacentNodes(const adjacency_list_range_iterator_t& adjacentIt) const;
      void constructLemonGraph(LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps);
      void constructHelperNodes(LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps, const adjacency_list_range_iterator_t& adjacentIt);
    private:
      LemonGraph m_graph;
      EmbeddingState& m_state;
      EmbeddingManager& m_embeddingManager;
      UnorderedMap<fuint32_t, LemonNode> m_nodeMap;
      UnorderedMap<edge_t, LemonArcPair, PairHashFunc<fuint32_t>> m_edgeMap;
      UnorderedSet<fuint32_t> m_mapped;

      LemonNode* m_s;
      LemonNode* m_t;
      capacity_t m_numberAdjacentLowered;
      fuint32_t m_sConnected;
  };

}


#endif