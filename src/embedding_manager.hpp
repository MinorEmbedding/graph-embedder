#ifndef __MAJORMINER_EMBEDDING_MANAGER_HPP
#define __MAJORMINER_EMBEDDING_MANAGER_HPP

#include "majorminer_types.hpp"

#include "common/embedding_state.hpp"

namespace majorminer
{
  enum ChangeType
  {
    DEL_MAPPING,
    INS_MAPPING,
    FREE_NEIGHBORS,
    OCCUPY_NODE,
    COMMIT
  };

  struct EmbeddingChange
  {
    EmbeddingChange()
      : m_type(ChangeType::COMMIT), m_a((fuint32_t)-1),
        m_b((fuint32_t)-1){}
    EmbeddingChange(ChangeType t, fuint32_t a)
      : m_type(t), m_a(a), m_b((fuint32_t)-1) {}
    EmbeddingChange(ChangeType t, fuint32_t a, fuint32_t b)
      : m_type(t), m_a(a), m_b(b) {}

    ChangeType m_type;
    fuint32_t m_a;
    fuint32_t m_b;
  };

  struct NodeAffected
  {
    NodeAffected()
      : m_timestampNodeChanged(0),
        m_timestampEdgeChanged(0) {}
    void clear()
    {
      m_timestampNodeChanged = 0;
      m_timestampEdgeChanged = 0;
    }

    fuint32_t m_timestampNodeChanged;
    fuint32_t m_timestampEdgeChanged;
  };

  class EmbeddingManager
  {
      friend EmbeddingSuite;
      friend SuperVertexPlacer;
      friend NetworkSimplexWrapper;
    public:
      EmbeddingManager(EmbeddingSuite& suite, EmbeddingState& state);
      void setFreeNeighbors(fuint32_t node, fuint32_t nbNeighbors);
      void deleteMappingPair(fuint32_t source, fuint32_t target);
      void insertMappingPair(fuint32_t source, fuint32_t target);
      void occupyNode(fuint32_t target);
      void synchronize();
      void commit();
      fuint32_t getTimestamp() { return m_time++; }
      int numberFreeNeighborsNeeded(fuint32_t sourceNode);
      const NodeAffected& getHistory(fuint32_t node) { return m_changeHistory[node]; }


      const embedding_mapping_t& getMapping() const { return m_mapping; }
      const embedding_mapping_t& getReverseMapping() const { return m_reverseMapping; }
      const nodeset_t& getNodesOccupied() const { return m_nodesOccupied; }
      const nodeset_t& getTargetNodesRemaining() const { return m_targetNodesRemaining; }
      const UnorderedMap<fuint32_t, std::atomic<int>>& getFreeNeighborMap() const { return m_sourceFreeNeighbors; }

      fuint32_t getLastNode() const { return m_lastNode; }

    private:
      void mapNode(fuint32_t source, fuint32_t target);
      void mapNode(fuint32_t source, const nodeset_t& targets);
      void clear();

    private:
      EmbeddingSuite& m_suite;
      EmbeddingState& m_state;
      embedding_mapping_t m_mapping;
      embedding_mapping_t m_reverseMapping;
      nodeset_t m_nodesOccupied;
      nodeset_t m_targetNodesRemaining;
      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceFreeNeighbors;
      Queue<EmbeddingChange> m_changesToPropagate;
      std::atomic<int> m_nbCommitsRemaining;

      UnorderedMap<fuint32_t, NodeAffected> m_changeHistory;

      std::atomic<fuint32_t> m_time;

      fuint32_t m_lastNode = (fuint32_t)-1;
  };



}



#endif