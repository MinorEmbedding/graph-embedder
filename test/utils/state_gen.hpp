#ifndef __MAJORMINER_STATE_GEN_HPP_
#define __MAJORMINER_STATE_GEN_HPP_

#include <majorminer_types.hpp>
#include <common/embedding_state.hpp>

namespace majorminer
{

  class StateGen
  {
    public:
      StateGen(const graph_t& source, const graph_t& target)
        : m_source(source), m_target(target) {}

      void addMapping(const embedding_mapping_t& mapping);
      void addMapping(std::initializer_list<edge_t> mapping);
      void addMapping(vertex_t source, std::initializer_list<vertex_t> targetVertices);
      std::unique_ptr<EmbeddingState> get();

    private:
      const graph_t& m_source;
      const graph_t& m_target;

      graph_t m_mapping;
  };

}


#endif