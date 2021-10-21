#ifndef __MAJORMINER_NETWORK_SIMPLEX_HPP_
#define __MAJORMINER_NETWORK_SIMPLEX_HPP_

#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>


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
    using LemonArcMap = SmartDigraph::ArcMap<T>;
    using NetworkSimplex = lemon::NetworkSimplex<LemonGraph, capacity_t, cost_t>;
    public:
      NetworkSimplexWrapper();

    private:
  };

}


#endif