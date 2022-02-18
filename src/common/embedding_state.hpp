#ifndef __MAJORMINER_EMBEDDING_STATE_HPP_
#define __MAJORMINER_EMBEDDING_STATE_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{

  class EmbeddingState
  {
    friend class EmbeddingManager;
    public:
      EmbeddingState(const graph_t& sourceGraph, const graph_t& targetGraph, EmbeddingVisualizer* vis);

      void mapNode(fuint32_t node, fuint32_t targetNode);
      void mapNode(fuint32_t source, const nodeset_t& targets);
      void updateNeededNeighbors(fuint32_t node);
      void updateConnections(fuint32_t node, PrioNodeQueue& nodesToProcess);
      int numberFreeNeighborsNeeded(fuint32_t sourceNode);
      fuint32_t getTrivialNode();

      bool removeRemainingNode(fuint32_t node);

      bool isNodeOccupied(fuint32_t node) const { return m_nodesOccupied.contains(node); }

    public: // getter
      const graph_t* getSourceGraph() const { return m_sourceGraph; }
      const graph_t* getTargetGraph() const { return m_targetGraph; }
      const adjacency_list_t& getSourceAdjGraph() const { return m_source; }
      const adjacency_list_t& getTargetAdjGraph() const { return m_target; }
      const embedding_mapping_t& getMapping() const { return m_mapping; }
      embedding_mapping_t& getMapping() { return m_mapping; }
      const embedding_mapping_t& getReverseMapping() const { return m_reverseMapping; }
      embedding_mapping_t& getReverseMapping() { return m_reverseMapping; }
      nodeset_t& getNodesOccupied() { return m_nodesOccupied; }
      const nodeset_t& getNodesOccupied() const { return m_nodesOccupied; }
      const nodeset_t& getRemainingTargetNodes() const { return m_targetNodesRemaining; }
      nodeset_t& getRemainingTargetNodes() { return m_targetNodesRemaining; }
      const nodeset_t& getNodesOccupied() const { return m_nodesOccupied; }
      const UnorderedMap<fuint32_t, fuint32_t> getRemainingNodes() const { return m_nodesRemaining; }
      UnorderedMap<fuint32_t, fuint32_t> getRemainingNodes() { return m_nodesRemaining; }

      EmbeddingVisualizer* getVisualizer() { return m_visualizer; }
      bool hasVisualizer() const { return m_visualizer != nullptr; }

      UnorderedMap<fuint32_t, std::atomic<int>>& getSourceFreeNeighbors() { return m_sourceFreeNeighbors; }
      UnorderedMap<fuint32_t, std::atomic<int>>& getSourceNeededNeighbors() { return m_sourceNeededNeighbors; }

    private:
      void initialize();


    private: // for friend EmbeddingManager


    private:
      const graph_t* m_sourceGraph;
      const graph_t* m_targetGraph;
      adjacency_list_t m_source;
      adjacency_list_t m_target;

      embedding_mapping_t m_mapping;
      embedding_mapping_t m_reverseMapping;
      nodeset_t m_nodesOccupied;
      nodeset_t m_targetNodesRemaining;

      UnorderedMap<fuint32_t, fuint32_t> m_nodesRemaining;
      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceNeededNeighbors;
      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceFreeNeighbors;
      nodeset_t m_sourceNodesAffected;

      EmbeddingVisualizer* m_visualizer;
  };

}



#endif
