#ifndef __MAJORMINER_LMRP_KING_SUGRAPH_HPP_
#define __MAJORMINER_LMRP_KING_SUGRAPH_HPP_

#include <majorminer_types.hpp>
#include <common/graph_info.hpp>
#include <lmrp/lmrp_subgraph.hpp>

namespace majorminer
{
  class KingLMRPSubgraph : public LMRPSubgraph
  {
    public:
      KingLMRPSubgraph(KingGraphInfo& g): m_graph(g) {}
      ~KingLMRPSubgraph() {}

      bool getSubgraph(vertex_t contained, nodeset_t& subgraph) override;

      bool isBeingDestroyed(vertex_t contained) override;

      void commit(vertex_t contained) override;

    private:
      bool checkOccupation(fuint32_t x, fuint32_t y) const;

      template<typename Functor>
      void neighborhood(fuint32_t x, fuint32_t y, Functor func) const
      {
        fuint32_t minX = static_cast<fuint32_t>(std::max(static_cast<int>(x) - 3, 0));
        fuint32_t maxX = std::min(x + 4, m_graph.getWidth());
        fuint32_t minY = static_cast<fuint32_t>(std::max(static_cast<int>(y) - 3, 0));
        fuint32_t maxY = std::min(y + 4, m_graph.getHeight());

        for (fuint32_t itX = minX; itX < maxX; ++itX)
        {
          bool craterX = ((itX + 3) != x) && (itX - 3 != x);
          for (fuint32_t itY = minY; itY < maxY; ++itY)
          {
            bool isInCrater = craterX && ((itY + 3) != y) && (itY - 3 != y);
            func(fuint32_pair_t{itX, itY}, isInCrater);
          }
        }
      }

    private:
      KingGraphInfo m_graph;
      coordinateset_t m_occupied;

      std::shared_mutex m_updateLock;
  };

}


#endif