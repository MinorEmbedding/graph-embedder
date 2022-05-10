#ifndef __MAJORMINER_EMBEDDING_ANALYZER_HPP_
#define __MAJORMINER_EMBEDDING_ANALYZER_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{
  class EmbeddingAnalyzer
  {
    public:
      EmbeddingAnalyzer(const embedding_mapping_t& emb)
        : m_embedding(emb) { }

      fuint32_t getNbOverlaps() const;
      fuint32_t getNbUsedNodes() const;


    private:
      const embedding_mapping_t& m_embedding;
  };
}


#endif