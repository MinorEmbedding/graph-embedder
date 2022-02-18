#ifndef __MAJORMINER_EMBEDDING_VALIDATOR_HPP_
#define __MAJORMINER_EMBEDDING_VALIDATOR_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{

  class EmbeddingValidator
  {
    public:
      EmbeddingValidator(const EmbeddingState& state)
       : m_state(state) {}

      bool isDisjoint() const;
      bool nodesConnected() const;
      bool isValid() const { return isDisjoint() && nodesConnected(); }

    private:
      void printMissingEdges(fuint32_t node) const;

    private:
      const EmbeddingState& m_state;
  };
}





#endif
