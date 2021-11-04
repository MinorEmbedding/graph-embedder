#ifndef __MAJORMINER_EXTEND_MUTATION_HPP_
#define __MAJORMINER_EXTEND_MUTATION_HPP_

#include "majorminer_types.hpp"
#include "majorminer.hpp"

#include "evolutionary/generic_mutation.hpp"


namespace majorminer
{
  class EmbeddingSuite;

  class MutationExtend : public GenericMutation
  {
    public:
      MutationExtend(EmbeddingSuite* suite, fuint32_t sourceNode);
      ~MutationExtend(){}
      void execute() override;
      bool isValid() override;
      bool prepare() override;

    private:
      double checkCandidate(fuint32_t extendNode, fuint32_t sourceNode, int delta);

    private:
      EmbeddingSuite& m_suite;
      nodeset_t m_degraded;
      fuint32_t m_sourceVertex;
      fuint32_t m_targetVertex;
      fuint32_t m_extendedTarget;
      bool m_valid = false;
  };


}



#endif
