#ifndef __MAJORMINER_GENERIC_MUTATION_HPP_
#define __MAJORMINER_GENERIC_MUTATION_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{
  class GenericMutation
  {
    public:
      virtual ~GenericMutation() {}

      // fast check whether a mutation is still valid
      virtual bool isValid() = 0;

      // Initial preparation or revalidating a preparation
      virtual bool prepare() = 0;

      // Incorporate changes of this mutation
      virtual void execute() = 0;

    protected:
      void generateTimestamp();

    private:
      fuint32_t m_timestamp;
  };
}


#endif
