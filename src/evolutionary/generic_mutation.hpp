#ifndef __MAJORMINER_GENERIC_MUTATION_HPP_
#define __MAJORMINER_GENERIC_MUTATION_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{
  class GenericMutation
  {
    public:
      virtual ~GenericMutation() {}
      virtual void execute() = 0;
  };
}


#endif
