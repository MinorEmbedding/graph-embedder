#ifndef __MAJORMINER_EXTEND_MUTATION_HPP_
#define __MAJORMINER_EXTEND_MUTATION_HPP_

#include "majorminer_types.hpp"
#include "evolutionary/generic_mutation.hpp"

namespace majorminer
{
  class MutationExtend : public GenericMutation
  {
    public:
      MutationExtend(const EmbeddingState& state, EmbeddingManager& embeddingManager, fuint32_t sourceNode);
      ~MutationExtend(){}
      void execute() override;
      bool isValid() override;
      bool prepare() override;

    private:
      double checkImprovement(fuint32_t extendNode, const EmbeddingBase& base);
      void updateFreeNeighbors();

    private:
      const EmbeddingState& m_state;
      EmbeddingManager& m_embeddingManager;

      nodeset_t m_degraded;
      fuint32_t m_sourceVertex;
      fuint32_t m_targetVertex;
      fuint32_t m_extendedTarget;
      bool m_improving = false;
      fuint32_t m_time;
  };


}



#endif
