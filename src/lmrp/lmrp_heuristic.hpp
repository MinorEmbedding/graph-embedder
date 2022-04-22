#ifndef __MAJORMINER_LMRP_HEURISTIC_HPP_
#define __MAJORMINER_LMRP_HEURISTIC_HPP_

#include <majorminer_types.hpp>
#include <lmrp/lmrp_types.hpp>

namespace majorminer
{

  class LMRPHeuristic
  {
    public:
      LMRPHeuristic(EmbeddingState& state, vertex_t target);
      void optimize();

    private:
      void buildBorder();
      void identifyEdges();
      void identifyComponents();
      void buildSubgraphs(graph_t& borderMapped, graph_t& subgraph);
      void calculatePreviousFitness();
      void identifyDestroyed();
      void solve();

      void connectComponent(ConnectedList& component, fuint32_t componentIdx);
      void embeddDestroyed(vertex_t destroyed);

      void addReachableComponent(vertex_t target, vertex_t source);
      void initializeDijkstraData();
      void resetDijkstra();
      void addSingleVertexNeighbors(vertex_t target, fuint32_t overlaps, fuint32_t length);
      void runDijkstraToTarget(nodeset_t& targets, vertex_t root);
      vertex_t checkConnectedTo(const nodeset_t& wantedTargets, vertex_t target);
      void addEmbeddedPath(vertex_t leaf);
      void connectAdjacentComponents(nodeset_t& adjacent);
      bool checkConnectedToSource(nodeset_t& wantedSources, vertex_t target);
      void addAllMapped();

      void mapVertex(vertex_t source, vertex_t target);

    private:
      const EmbeddingState& m_state;
      bool m_done;
      fuint32_t m_numberOverlaps;
      fuint32_t m_numberMapped;

      nodeset_t m_crater;
      nodeset_t m_border;
      nodeset_t m_completelyDestroyed;
      graph_t m_edges;
      adjacency_list_t m_sourceAdjacencies;

      Vector<ConnectedList> m_componentsList;
      Vector<vertex_t> m_componentVertices;

      embedding_mapping_t m_mapping;
      embedding_mapping_t m_reverse;
      graph_t m_superVertices;

      PriorityQueue<DijkstraVertex> m_dijkstraQueue;
      UnorderedMap<vertex_t, DijkstraVertex> m_bestPaths;
      fuint32_t m_currentSource;
  };


}

#endif
