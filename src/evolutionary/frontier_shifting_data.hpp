#ifndef __MAJORMINER_FRONTIER_SHIFTING_DATA_HPP_
#define __MAJORMINER_FRONTIER_SHIFTING_DATA_HPP_

#include "majorminer_types.hpp"

namespace majorminer
{

  struct FrontierShiftingData
  {
    FrontierShiftingData() : m_victimSourceNode((fuint32_t)-1) { }
    void clear();
    void setNbNodes(fuint32_t nbNodes);
    void setVictimSource(fuint32_t node);
    void lostNode(fuint32_t conquered, fuint32_t conquerorSource);
    void addConnection(fuint32_t sourceFrom, fuint32_t targetTo);
    void findCutVertices();
    bool isNowACutVertex(fuint32_t contested) const;


    fuint32_t m_victimSourceNode;
    fuint32_t m_nbNodes;
    adjacency_list_t m_victimSubgraph;
    nodeset_t m_cutVertices;

    // source node (connected to victim) --> target node mapped to from victim
    adjacency_list_t m_victimConnections; // all connections other nodes have to the victim's chain
    adjacency_list_t m_reverseConnections; // reverse m_victimConnections
  };

}

#endif