#ifndef __MAJORMINER_LMRP_CHIMERA_SUBGRAPH_HPP_
#define __MAJORMINER_LMRP_CHIMERA_SUBGRAPH_HPP_

#include <majorminer_types.hpp>
#include <lmrp/lmrp_subgraph.hpp>
#include <common/graph_info.hpp>

namespace majorminer
{

  class ChimeraLMRPSubgraph : public LMRPSubgraph
  {
    public:
      ChimeraLMRPSubgraph(ChimeraGraphInfo& g): m_graph(g) {}
      ~ChimeraLMRPSubgraph() {}

      bool getSubgraph(vertex_t contained, nodeset_t& subgraph) override;

      bool isBeingDestroyed(vertex_t contained) override;

      // If a LMRP instance was solved, free the mapped vertices
      void commit(vertex_t contained) override;

    private:
      bool checkOccupation(fuint32_t x, fuint32_t y, vertex_t contained) const;

      template<typename Functor>
      void neighborhood(fuint32_t x, fuint32_t y, vertex_t contained, Functor func) const
      {
        bool leftSide = (contained & 0x04) == 0;
        auto width  = m_graph.getWidth();
        auto height = m_graph.getHeight();

        fuint32_t minX = static_cast<fuint32_t>(std::max(static_cast<int>(x) - (leftSide ? 2 : 1), 0));
        fuint32_t maxX = std::min(x + (leftSide ? 2 : 3), width);
        fuint32_t minY = y > 0 ? (y - 1) : 0;
        fuint32_t maxY = std::min(y + 3, height);

        fuint32_t otherX = (!leftSide ? (x + 1) : (x > 0 ? x - 1 : FUINT32_UNDEF));

        for (fuint32_t itX = minX; itX < maxX; ++itX)
        {
          for (fuint32_t itY = minY; itY < maxY; ++itY)
          {
            bool craterY = (itY == y || itY == y + 1);
            bool isInCrater = craterY && (itX == x || itX == otherX);
            func(fuint32_pair_t{itX, itY}, isInCrater);
          }
        }
      }

    private:
      ChimeraGraphInfo m_graph;
      coordinateset_t m_occupied;

      std::shared_mutex m_updateLock;
  };

}


#endif