#ifndef __MAJORMINER_MUTATION_FRONTIER_SHIFTING_HPP_
#define __MAJORMINER_MUTATION_FRONTIER_SHIFTING_HPP_

#include "majorminer_types.hpp"

#include "evolutionary/generic_mutation.hpp"

namespace majorminer
{
  class MutationFrontierShifting : public GenericMutation
  {
    public:
      MutationFrontierShifting(const EmbeddingState& state, EmbeddingManager& manager, fuint32_t conquerorSource);
      ~MutationFrontierShifting() {}

      void execute() override;
      bool isValid() override;
      bool prepare() override;
      fuint32_t getConqueror() const { return m_conqueror; }
      fuint32_t getVictim() const { return m_victim; }
      fuint32_t getContested() const { return m_bestContested; }

    private:
      double calculateImprovement(fuint32_t victim);

    private:
      const EmbeddingState& m_state;
      EmbeddingManager& m_manager;
      fuint32_t m_conqueror;
      fuint32_t m_victim;
      fuint32_t m_bestContested;
      double m_bestImprovement = MAXFLOAT;
      bool m_valid;
  };
}


#endif