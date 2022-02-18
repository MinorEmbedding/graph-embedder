#ifndef __MAJORMINER_MUTATION_SIMPLE_REDUCE_OVERLAP_HPP_
#define __MAJORMINER_MUTATION_SIMPLE_REDUCE_OVERLAP_HPP_

#include <majorminer_types.hpp>
#include <evolutionary/generic_mutation.hpp>

namespace majorminer
{
  class MutationReduceOverlap : public GenericMutation
  {
    public:
      MutationReduceOverlap(EmbeddingState& state, EmbeddingManager& manager)
        : m_state(state), m_manager(manager) {}

    private:
      EmbeddingState& m_state;
      EmbeddingManager& m_manager;

  };
}


#endif