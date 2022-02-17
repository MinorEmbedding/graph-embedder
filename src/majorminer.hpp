#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include "config.hpp"
#include "embedding_manager.hpp"
#include "embedding_state.hpp"

#include "common/graph_gen.hpp"
#include "common/utils.hpp"
#include "common/embedding_validator.hpp"
#include "common/embedding_visualizer.hpp"
#include "common/cut_vertex.hpp"

#include "initial/super_vertex_placer.hpp"

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
      void prepareFrontierShifting(fuint32_t victimNode, fuint32_t nbConnectedTo);

      void identifyAffected(fuint32_t node);


      // query the delta between the number of free nodes
      // and the needed amount of by the sourceNode
      double checkExtendCandidate(fuint32_t extendNode, fuint32_t sourceNode, int delta);

      void tryMutations();

    private:
      EmbeddingState m_state;
      SuperVertexPlacer& m_placer;
      EmbeddingManager m_embeddingManager;
      MutationManager m_mutationManager;

      // TODO: change to priority queue depending on fitness function
      Queue<std::unique_ptr<GenericMutation>> m_taskQueue;

      FrontierShiftingData m_frontierData;

      EmbeddingVisualizer* m_visualizer;
  };

}



#endif