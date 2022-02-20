#ifndef __MAJORMINER_EMBEDDING_BASE_HPP_
#define __MAJORMINER_EMBEDDING_BASE_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  class EmbeddingBase
  {
    public:
      virtual ~EmbeddingBase(){}

    // getters
      virtual const graph_t* getSourceGraph() const = 0;
      virtual const graph_t* getTargetGraph() const = 0;
      virtual const adjacency_list_t& getSourceAdjGraph() const = 0;
      virtual const adjacency_list_t& getTargetAdjGraph() const = 0;
      virtual const embedding_mapping_t& getMapping() const = 0;
      virtual const embedding_mapping_t& getReverseMapping() const = 0;
      virtual const nodeset_t& getNodesOccupied() const = 0;
      virtual const nodeset_t& getRemainingTargetNodes() const = 0;


      bool isTargetNodeOccupied(fuint32_t targetNode) const { return getRemainingTargetNodes().contains(targetNode); }
      virtual int numberFreeNeighborsNeeded(fuint32_t sourceNode) const = 0;

    // Iteration methods
    public:

      // For a given sourceNode, iterate over all target nodes sourceNode is mapped onto.
      // For these target nodes, iterate over all their neighbors
      // but skip a neighbor if skipOccupied && isOccupied(neighbor)
      // Callback functor should take as parameters
      // 1. fuint32_t target neighbor
      // 2. fuint32_t target sourceNode is mapped onto
      template<bool skipOccupied, typename Functor>
      void iterateSourceMappingAdjacent(fuint32_t sourceNode, Functor func) const
      {
        auto embeddedPathIt = getMapping().equal_range(sourceNode);
        const auto& target = getTargetAdjGraph();
        const auto& occupied = getNodesOccupied();

        for (auto targetNode = embeddedPathIt.first; targetNode != embeddedPathIt.second; ++targetNode)
        {
          // find nodes that are adjacent to targetNode (in the targetGraph)
          auto targetGraphAdjacentIt = target.equal_range(targetNode->second);
          for (auto targetAdjacent = targetGraphAdjacentIt.first; targetAdjacent != targetGraphAdjacentIt.second; ++targetAdjacent)
          {
            if (skipOccupied && occupied.contains(targetAdjacent->second)) continue;
            if (func(targetAdjacent->second, targetNode->second)) return;
          }
        }
      }

      template<typename Functor>
      void iterateReverseMapping(fuint32_t mappedTargetNode, Functor func) const
      {
        auto revRange = getReverseMapping().equal_range(mappedTargetNode);
        for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
        {
          if (func(revIt->second)) return;
        }
      }

      template<typename Functor>
      void iterateTargetGraphAdjacent(fuint32_t targetNode, Functor func) const
      {
        auto adjRange = getTargetAdjGraph().equal_range(targetNode);
        for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
        {
          if (func(adj->second)) return;
        }
      }
  };


}




#endif