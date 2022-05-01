#ifndef __MAJORMINER_SUPER_VERTEX_PLACER_HPP_
#define __MAJORMINER_SUPER_VERTEX_PLACER_HPP_

#include <majorminer_types.hpp>
#include <initial/network_simplex.hpp>

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
      void replaceOverlapping();

    private:
      void identifyOverlapping(nodeset_t& overlapping);
      void improveMapping(vertex_t source);
      void trivialNode();
      bool connectedNode();
      void replaceSuperVertex(vertex_t source, nodeset_t& svertex);

      void embeddNode(vertex_t node);
      void embeddNodeNetworkSimplex(vertex_t node, const nodeset_t* oldMapping = nullptr);

      void embeddTrivialNode(vertex_t node);
      void embeddSimpleNode(vertex_t node);

      void visualize(vertex_t node, PlacedNodeType type, fuint32_t nbConnections = 0);

    private:
      EmbeddingState& m_state;
      EmbeddingManager& m_embeddingManager;

      PrioNodeQueue m_nodesToProcess;

      std::unique_ptr<NetworkSimplexWrapper> m_nsWrapper;
  };

}



#endif