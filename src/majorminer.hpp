#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include "config.hpp"
#include "embedding_manager.hpp"
#include "common/embedding_state.hpp"
#include "initial/super_vertex_placer.hpp"
#include "evolutionary/muation_manager.hpp"

namespace majorminer
{
  class EmbeddingSuite
  {
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
      SuperVertexPlacer m_placer;
      EmbeddingManager m_embeddingManager;
      MutationManager m_mutationManager;

      // TODO: change to priority queue depending on fitness function
      Queue<std::unique_ptr<GenericMutation>> m_taskQueue;

      // FrontierShiftingData m_frontierData;

      EmbeddingVisualizer* m_visualizer;
  };

}



#endif