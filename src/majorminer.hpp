#ifndef __MAJORMINER_MAJOR_MINER_HPP_
#define __MAJORMINER_MAJOR_MINER_HPP_

#include<tbb/tbb.h>
#include<tbb/parallel_sort.h>
#include<vector>
#include<iostream>
#include<Eigen/Sparse>
#include <memory>

#include "config.hpp"
#include "graph_gen.hpp"
#include "utils.hpp"
#include "network_simplex.hpp"
#include "embedding_validator.hpp"

namespace majorminer
{
  class NetworkSimplexWrapper;

  class EmbeddingSuite
  {
    friend NetworkSimplexWrapper;
    public:
      EmbeddingSuite(const graph_t& source, const graph_t& target);

      embedding_mapping_t find_embedding();
      bool isValid() const;
      bool connectsNodes() const;

    private:
      void embeddNode(fuint32_t node);
      void embeddNodeNetworkSimplex(fuint32_t node);

      void mapNode(fuint32_t node, fuint32_t targetNode);
      void mapNode(fuint32_t node, const nodeset_t& targetNodes);
      void updateConnections(fuint32_t node);

      void embeddTrivialNode(fuint32_t node);
      void embeddSimpleNode(fuint32_t node);

    private:
      const graph_t* m_sourceGraph;
      const graph_t* m_targetGraph;
      adjacency_list_t m_source;
      adjacency_list_t m_target;

      embedding_mapping_t m_mapping;
      nodeset_t m_nodesOccupied;
      nodeset_t m_targetNodesRemaining;
      UnorderedMap<fuint32_t, fuint32_t> m_nodesRemaining;
      PrioNodeQueue m_nodesToProcess;

      std::unique_ptr<NetworkSimplexWrapper> m_nsWrapper;
  };

}



#endif