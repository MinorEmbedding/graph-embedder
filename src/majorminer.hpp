#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include<tbb/tbb.h>

#include "config.hpp"
#include "embedding_manager.hpp"
#include "common/graph_gen.hpp"
#include "common/utils.hpp"
#include "common/embedding_validator.hpp"
#include "common/embedding_visualizer.hpp"
#include "common/cut_vertex.hpp"

#include "initial/network_simplex.hpp"

#include "evolutionary/mutation_extend.hpp"
#include "evolutionary/frontier_shifting_data.hpp"
#include "evolutionary/mutation_frontier_shifting.hpp"
#include "evolutionary/muation_manager.hpp"

namespace majorminer
{
  class NetworkSimplexWrapper;
  class GenericMutation;
  class MutationExtend;
  class MuationFrontierShifting;
  class EmbeddingManager;
  class MutationManager;
  struct FrontierShiftingData;

  class EmbeddingSuite
  {
      friend NetworkSimplexWrapper;
      friend MutationExtend;
      friend MuationFrontierShifting;
      friend EmbeddingManager;
      friend MutationManager;
    public:
      EmbeddingSuite(const graph_t& source, const graph_t& target, EmbeddingVisualizer* visualizer = nullptr);

      embedding_mapping_t find_embedding();
      bool isValid() const;
      bool connectsNodes() const;

    private:
      void embeddNode(fuint32_t node);
      void embeddNodeNetworkSimplex(fuint32_t node);

      void updateConnections(fuint32_t node);

      void prepareFrontierShifting(fuint32_t victimNode, fuint32_t nbConnectedTo);

      void embeddTrivialNode(fuint32_t node);
      void embeddSimpleNode(fuint32_t node);
      void identifyAffected(fuint32_t node);

      void updateNeededNeighbors(fuint32_t node);

      // query the delta between the number of free nodes
      // and the needed amount of by the sourceNode
      int numberFreeNeighborsNeeded(fuint32_t sourceNode);
      double checkExtendCandidate(fuint32_t extendNode, fuint32_t sourceNode, int delta);
      void tryMutations();
      void mapNode(fuint32_t source, const nodeset_t& targets);

    private:
      const graph_t* m_sourceGraph;
      const graph_t* m_targetGraph;
      adjacency_list_t m_source;
      adjacency_list_t m_target;

      EmbeddingManager m_embeddingManager;
      MutationManager m_mutationManager;

      embedding_mapping_t m_mapping;
      embedding_mapping_t m_reverseMapping;
      nodeset_t m_nodesOccupied;
      nodeset_t m_targetNodesRemaining;
      UnorderedMap<fuint32_t, fuint32_t> m_nodesRemaining;
      PrioNodeQueue m_nodesToProcess;

      // TODO: change to priority queue depending on fitness function
      Queue<std::unique_ptr<GenericMutation>> m_taskQueue;

      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceNeededNeighbors;
      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceFreeNeighbors;
      nodeset_t m_sourceNodesAffected;

      FrontierShiftingData m_frontierData;

      std::unique_ptr<NetworkSimplexWrapper> m_nsWrapper;
      EmbeddingVisualizer* m_visualizer;
  };

}



#endif