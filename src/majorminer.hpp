#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include <common/embedding_manager.hpp>
#include <common/embedding_state.hpp>
#include <initial/super_vertex_placer.hpp>
#include <evolutionary/mutation_manager.hpp>

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
      void finishVisualization();

    private:
      EmbeddingState m_state;
      EmbeddingVisualizer* m_visualizer;
      EmbeddingManager m_embeddingManager;
      MutationManager m_mutationManager;
      SuperVertexPlacer m_placer;

      Queue<std::unique_ptr<GenericMutation>> m_taskQueue;
  };

}



#endif