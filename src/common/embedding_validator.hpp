#ifndef __MAJORMINER_EMBEDDING_VALIDATOR_HPP_
#define __MAJORMINER_EMBEDDING_VALIDATOR_HPP_

#include "majorminer_types.hpp"
#include "utils.hpp"

namespace majorminer
{

  class EmbeddingValidator
  {
    public:
      EmbeddingValidator(const embedding_mapping_t& embedding,
      const graph_t& source, const adjacency_list_t& target)
       : m_embedding(embedding), m_source(source), m_target(target){}

      bool isDisjoint() const;
      bool nodesConnected() const;
      bool isValid() const { return isDisjoint() && nodesConnected(); }

    private:
      const embedding_mapping_t& m_embedding;
      const graph_t& m_source;
      const adjacency_list_t& m_target;
  };

}





#endif
