#ifndef __MAJORMINER_LMRP_HEURISTIC_HPP_
#define __MAJORMINER_LMRP_HEURISTIC_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  class LMRPHeuristic
  {
    public:
      LMRPHeuristic(const EmbeddingState& state);

    private:
      

      fuint32_t m_numberOverlaps;
      fuint32_t m_numberMapped;
  };


}

#endif
