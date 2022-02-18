#ifndef __MAJORMINER_MUTATION_MANAGER_HPP_
#define __MAJORMINER_MUTATION_MANAGER_HPP_

#include <thread>
#include <mutex>

#include "majorminer_types.hpp"

#include "evolutionary/generic_mutation.hpp"
#include "common/embedding_state.hpp"
#include "embedding_manager.hpp"

namespace majorminer
{
  class MutationManager
  {
    typedef std::unique_ptr<GenericMutation> MutationPtr;

    public:
      MutationManager(EmbeddingState& state, EmbeddingManager& embeddingManager)
        : m_state(state), m_embeddingManager(embeddingManager) {}

      void mutate();

    private:
      void prepare();
      void incorporate();
      void prepareAffectedExtendCandidates(fuint32_t node);

    private:
      EmbeddingState& m_state;
      EmbeddingManager& m_embeddingManager;
      Queue<MutationPtr> m_prepQueue;
      PriorityQueue<MutationPtr> m_incorporationQueue;
      std::atomic<bool> m_done;
      std::atomic<fuint32_t> m_runningPreps;
      std::atomic<bool> m_wait;
      std::mutex m_free;
  };

}


#endif