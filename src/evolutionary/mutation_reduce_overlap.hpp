#ifndef __MAJORMINER_MUTATION_SIMPLE_REDUCE_OVERLAP_HPP_
#define __MAJORMINER_MUTATION_SIMPLE_REDUCE_OVERLAP_HPP_

#include <majorminer_types.hpp>
#include <evolutionary/generic_mutation.hpp>

namespace majorminer
{
  class MutationReduceOverlap : public GenericMutation
  {
    public:
      MutationReduceOverlap(EmbeddingState& state, EmbeddingManager& manager, fuint32_t sourceVertex);
      ~MutationReduceOverlap();

      bool prepare() override;
      void execute() override;
      bool isValid() override;

      bool requeue() const override { return true; }

    private:
      EmbeddingState& m_state;
      EmbeddingManager& m_manager;
      SuperVertexReducer* m_reducer;
      fuint32_t m_sourceVertex;
  };
}


#endif