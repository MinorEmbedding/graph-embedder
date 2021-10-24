#ifndef __MAJORMINER_NETWORK_SIMPLEX_HPP_
#define __MAJORMINER_NETWORK_SIMPLEX_HPP_

#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>

#include "majorminer_types.hpp"
#include "majorminer.hpp"

namespace majorminer
{
  class EmbeddingSuite;

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
    public:
      NetworkSimplexWrapper(EmbeddingSuite* suite)
        : m_suite(suite) { }

      void embeddNode(fuint32_t node);

    private:
      LemonNode createNode(fuint32_t node);
      cost_t determineCost(fuint32_t node);
      void clear();

    private:
      LemonGraph m_graph;
      EmbeddingSuite* m_suite;
      UnorderedMap<fuint32_t, LemonNode> m_nodeMap;
      UnorderedMap<edge_t, std::pair<LemonArc, LemonArc>, PairHashFunc<fuint32_t>> m_edgeMap;
      UnorderedSet<fuint32_t> m_mapped;
  };

}


#endif