#ifndef __MAJORMINER_MUTATION_FRONTIER_SHIFTING_HPP_
#define __MAJORMINER_MUTATION_FRONTIER_SHIFTING_HPP_

#include "evolutionary/generic_mutation.hpp"
#include "majorminer_types.hpp"
#include "majorminer.hpp"


namespace majorminer
{

  class MuationFrontierShifting : public GenericMutation
  {
    public:
      MuationFrontierShifting(EmbeddingSuite* suite, fuint32_t conquerorSource, fuint32_t victimSource);
      ~MuationFrontierShifting() {}

      void execute() override;

    private:
      bool isCrucial(fuint32_t candidateNode);
      double calculateImprovement(fuint32_t candidateNode);

    private:
      EmbeddingSuite& m_suite;
      fuint32_t m_conqueror;
      fuint32_t m_victim;
      fuint32_t m_bestContested;
      double m_bestImprovement = MAXFLOAT;
      bool m_valid;
  };
}


#endif