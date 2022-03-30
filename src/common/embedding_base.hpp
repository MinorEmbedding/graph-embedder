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


      bool isTargetNodeOccupied(vertex_t targetNode) const { return getRemainingTargetNodes().contains(targetNode); }
      virtual int numberFreeNeighborsNeeded(vertex_t sourceNode) const = 0;

    // Iteration methods
    public:

      // For a given sourceNode, iterate over all target nodes sourceNode is mapped onto.
      // For these target nodes, iterate over all their neighbors
      // but skip a neighbor if skipOccupied && isOccupied(neighbor)
      // Callback functor should take as parameters
      // 1. fuint32_t target neighbor
      // 2. fuint32_t target sourceNode is mapped onto
      template<bool skipOccupied, typename Functor>
      void iterateSourceMappingAdjacent(vertex_t sourceNode, Functor func) const
      {
        auto embeddedPathIt = getMapping().equal_range(sourceNode);
        const auto& target = getTargetAdjGraph();
        const auto& remaining = getRemainingTargetNodes();

        for (auto targetNode = embeddedPathIt.first; targetNode != embeddedPathIt.second; ++targetNode)
        {
          // find nodes that are adjacent to targetNode (in the targetGraph)
          auto targetGraphAdjacentIt = target.equal_range(targetNode->second);
          for (auto targetAdjacent = targetGraphAdjacentIt.first; targetAdjacent != targetGraphAdjacentIt.second; ++targetAdjacent)
          {
            if (skipOccupied && !remaining.contains(targetAdjacent->second)) continue;
            if (func(targetAdjacent->second, targetNode->second)) return;
          }
        }
      }

      template<typename Functor>
      void iterateReverseMapping(vertex_t mappedTargetNode, Functor func) const
      {
        auto revRange = getReverseMapping().equal_range(mappedTargetNode);
        for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
        {
          func(revIt->second);
        }
      }

      // Iterate for a sourceNode over all target nodes source Node is mapped to.
      // For these target nodes iterate over their adjacent nodes and for the adjacent nodes
      // iterate over their reverse mapping. Awful everything.
      template<typename Functor>
      void iterateSourceMappingAdjacentReverse(vertex_t sourceNode, vertex_t skipTarget, Functor func) const
      {
        auto embeddedPathIt = getMapping().equal_range(sourceNode);
        const auto& target = getTargetAdjGraph();
        const auto& reverseMapping = getReverseMapping();

        for (auto mapIt = embeddedPathIt.first; mapIt != embeddedPathIt.second; ++mapIt)
        {
          if (mapIt->second == skipTarget) continue;
          auto targetGraphAdjacentIt = target.equal_range(mapIt->second);
          for (auto targetAdjacent = targetGraphAdjacentIt.first; targetAdjacent != targetGraphAdjacentIt.second; ++targetAdjacent)
          {
            auto revRange = reverseMapping.equal_range(targetAdjacent->second);
            for (auto revIt = revRange.first; revIt != revRange.second; ++revIt)
            {
              if (revIt->second != sourceNode && func(revIt->second)) return;
            }
          }
        }
      }

      template<typename Functor>
      void iterateTargetGraphAdjacent(vertex_t targetNode, Functor func) const
      {
        auto adjRange = getTargetAdjGraph().equal_range(targetNode);
        for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
        {
          func(adj->second);
        }
      }

      template<typename Functor>
      void iterateSourceGraphAdjacent(vertex_t sourceNode, Functor func) const
      {
        auto adjRange = getSourceAdjGraph().equal_range(sourceNode);
        for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
        {
          func(adj->second);
        }
      }

      template<typename Functor>
      void iterateSourceGraphAdjacentBreak(vertex_t sourceNode, Functor func) const
      {
        auto adjRange = getSourceAdjGraph().equal_range(sourceNode);
        for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
        {
          if (func(adj->second)) return;
        }
      }

      template<typename Functor>
      void iterateTargetGraphAdjacentBreak(vertex_t targetNode, Functor func) const
      {
        auto adjRange = getTargetAdjGraph().equal_range(targetNode);
        for (auto adj = adjRange.first; adj != adjRange.second; ++adj)
        {
          if (func(adj->second)) return;
        }
      }

      template<typename Functor>
      void iterateTargetAdjacentReverseMapping(vertex_t target, Functor func) const
      {
        const auto& targetGraph = getTargetAdjGraph();
        const auto& reverse = getReverseMapping();
        auto adjacentRange = targetGraph.equal_range(target);
        for (auto adjIt = adjacentRange.first; adjIt != adjacentRange.second; ++adjIt)
        {
          auto revMappedRange = reverse.equal_range(adjIt->second);
          for (auto revIt = revMappedRange.first; revIt != revMappedRange.second; ++revIt)
          {
            func(revIt->second);
          }
        }
      }


      /// For a given source node, iterates over every pair of two target nodes the source node
      /// is mapped to and invokes the functor func using this pair of target nodes.
      template<typename Functor>
      void iterateSourceMappingPair(vertex_t sourceVertex, Functor func) const
      {
        const auto& mapping = getMapping();
        auto mappedRange = mapping.equal_range(sourceVertex);
        for (auto mapped1It = mappedRange.first; mapped1It != mappedRange.second; ++mapped1It)
        {
          auto mapped2It = mapped1It;
          mapped2It++;

          for (; mapped2It != mappedRange.second; ++mapped2It)
          {
            func(mapped1It->second, mapped2It->second);
          }
        }
      }

      template<typename Functor>
      void iterateSourceMapping(vertex_t sourceVertex, Functor func) const
      {
        const auto& mapping = getMapping();
        auto mappingRange = mapping.equal_range(sourceVertex);
        for (auto it = mappingRange.first; it != mappingRange.second; ++it)
        {
          func(it->second);
        }
      }

      template<typename Functor>
      void iterateFreeTargetAdjacent(vertex_t targetVertex, Functor func) const
      {
        const auto& remaining = getRemainingTargetNodes();
        const auto& targetGraph = getTargetAdjGraph();
        auto range = targetGraph.equal_range(targetVertex);
        for (auto it = range.first; it != range.second; ++it)
        {
          if (remaining.contains(it->second)) func(it->second);
        }
      }
  };


}




#endif