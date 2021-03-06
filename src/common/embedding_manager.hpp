#ifndef __MAJORMINER_EMBEDDING_MANAGER_HPP
#define __MAJORMINER_EMBEDDING_MANAGER_HPP

#include <majorminer_types.hpp>
#include <common/embedding_base.hpp>
#include <common/random_gen.hpp>

namespace majorminer
{
  enum ChangeType
  {
    DEL_MAPPING,
    INS_MAPPING,
    FREE_NEIGHBORS,
    OCCUPY_NODE,
    COMMIT,
    FREE_NODE
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

  class EmbeddingManager : public EmbeddingBase
  {
      friend SuperVertexPlacer;
      friend NetworkSimplexWrapper;
      friend MutationManager;
    public:
      EmbeddingManager(EmbeddingSuite& suite, EmbeddingState& state);
      void setFreeNeighbors(vertex_t node, fuint32_t nbNeighbors);
      void deleteMappingPair(vertex_t source, vertex_t target);
      void insertMappingPair(vertex_t source, vertex_t target);
      void occupyNode(vertex_t target);
      void freeNode(vertex_t target);
      void synchronize();
      void commit();
      fuint32_t getTimestamp() { return m_time++; }
      int numberFreeNeighborsNeeded(vertex_t sourceNode) const override;
      const NodeAffected& getHistory(vertex_t node) { return m_changeHistory[node]; }

      const UnorderedMap<fuint32_t, std::atomic<int>>& getFreeNeighborMap() const { return m_sourceFreeNeighbors; }

      fuint32_t getLastNode() const { return m_lastNode; }

      ShiftingCandidates getCandidatesFor(vertex_t conquerorNode);
      ShiftingCandidates setCandidatesFor(vertex_t conquerorNode, nodepairset_t& candidates);

      RandomGen& getRandomGen() { return m_random; }

      EmbeddingVisualizer* getVisualizer();

    public:
      const graph_t* getSourceGraph() const override;
      const graph_t* getTargetGraph() const override;
      const adjacency_list_t& getSourceAdjGraph() const override;
      const adjacency_list_t& getTargetAdjGraph() const override;
      const embedding_mapping_t& getMapping() const override;
      const embedding_mapping_t& getReverseMapping() const override;
      const nodeset_t& getNodesOccupied() const override;
      const nodeset_t& getRemainingTargetNodes() const override;


    private:
      void unmapNode(vertex_t sourceVertex);
      void mapNode(vertex_t source, vertex_t target);
      void mapNode(vertex_t source, const nodeset_t& targets);
      void clear();

    private:
      EmbeddingSuite& m_suite;
      EmbeddingState& m_state;
      CandidateCache m_candidateCache;
      RandomGen m_random;

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