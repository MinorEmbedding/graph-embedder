#ifndef __MAJORMINER_MUTATION_MANAGER_HPP_
#define __MAJORMINER_MUTATION_MANAGER_HPP_

#include <thread>

#include "majorminer_types.hpp"

#include "evolutionary/mutation_extend.hpp"
#include "evolutionary/mutation_frontier_shifting.hpp"



namespace majorminer
{
  class EmbeddingSuite;

  class MutationManager
  {
    typedef std::unique_ptr<GenericMutation> MutationPtr;

    public:
      MutationManager(EmbeddingSuite& suite)
        : m_suite(suite) {}

      void mutate();

    private:
      void prepare();
      void incorporate();
      void prepareAffectedExtendCandidates(fuint32_t node);

    private:
      EmbeddingSuite& m_suite;
      Queue<MutationPtr> m_prepQueue;
      PriorityQueue<MutationPtr> m_incorporationQueue;
  };

}


#endif