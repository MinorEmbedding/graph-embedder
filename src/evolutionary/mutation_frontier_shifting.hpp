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
  };
}


#endif