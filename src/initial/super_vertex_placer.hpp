#ifndef __MAJORMINER_SUPER_VERTEX_PLACER_HPP_
#define __MAJORMINER_SUPER_VERTEX_PLACER_HPP_

#include "majorminer_types.hpp"
#include "embedding_state.hpp"
#include "embedding_manager.hpp"

#include "initial/network_simplex.hpp"


namespace majorminer
{
  class SuperVertexPlacer
  {
    enum PlacedNodeType
    {
      TRIVIAL, SIMPLE, COMPLEX
    };
    public:
      SuperVertexPlacer(EmbeddingState& state, EmbeddingManager& embeddingManager);

      void operator()();

    private:
      void trivialNode();
      bool connectedNode();

      void embeddNode(fuint32_t node);
      void embeddNodeNetworkSimplex(fuint32_t node);

      void embeddTrivialNode(fuint32_t node);
      void embeddSimpleNode(fuint32_t node);

      void visualize(fuint32_t node, PlacedNodeType type, fuint32_t nbConnections = 0);

    private:
      EmbeddingState& m_state;
      EmbeddingManager& m_embeddingManager;

      PrioNodeQueue m_nodesToProcess;

      std::unique_ptr<NetworkSimplexWrapper> m_nsWrapper;
  };

}



#endif