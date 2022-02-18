#ifndef __MAJORMINER_MUTATION_FRONTIER_SHIFTING_HPP_
#define __MAJORMINER_MUTATION_FRONTIER_SHIFTING_HPP_

#include "majorminer_types.hpp"

#include "evolutionary/generic_mutation.hpp"

#include "common/embedding_state.hpp"

namespace majorminer
{
  class MuationFrontierShifting : public GenericMutation
  {
    public:
      MuationFrontierShifting(EmbeddingSuite* suite, fuint32_t conquerorSource, fuint32_t victimSource);
      ~MuationFrontierShifting() {}

      void execute() override;
      fuint32_t getConqueror() const { return m_conqueror; }
      fuint32_t getVictim() const { return m_victim; }
      fuint32_t getContested() const { return m_bestContested; }

    private:
      bool isCrucial(fuint32_t candidateNode);
      double calculateImprovement(fuint32_t candidateNode);

    private:
      const EmbeddingState& m_state;
      fuint32_t m_conqueror;
      fuint32_t m_victim;
      fuint32_t m_bestContested;
      double m_bestImprovement = MAXFLOAT;
      bool m_valid;
  };
}


#endif