#include "frontier_shifting_data.hpp"

using namespace majorminer;



void FrontierShiftingData::clear()
{
  m_victimSourceNode = -1;
  m_victimSubgraph.clear();
  m_cutVertices.clear();
  m_victimConnections.clear();
  m_reverseConnections.clear();
}

void FrontierShiftingData::lostNode(fuint32_t conquered, fuint32_t conquerorSource)
{
  if (m_nbNodes <= 1) throw std::runtime_error("Bad number of nodes in frontiershifting data.");
  m_nbNodes--;
  m_cutVertices.clear();
  auto range = m_reverseConnections.equal_range(conquered);
  for (auto it = range.first; it != range.second; ++it)
  {
    eraseSinglePair(m_victimConnections, it->second, it->first);
  }
  m_reverseConnections.unsafe_erase(range.first, range.second);
  nodeset_t newConnections{};
  for (auto it = m_victimSubgraph.begin(); it !=m_victimSubgraph.end();)
  {
    if(it->first == conquered)
    {
      m_victimConnections.insert(edge_t{conquerorSource, it->second});
      m_reverseConnections.insert(edge_t{it->second, conquerorSource});
    }
    if (it->first == conquered || it->second == conquered)
    {
      it = m_victimSubgraph.unsafe_erase(it);
    }
    else ++it;
  }
}

void FrontierShiftingData::addConnection(fuint32_t sourceFrom, fuint32_t targetTo)
{
  m_victimConnections.insert(edge_t{ sourceFrom, targetTo });
  m_reverseConnections.insert(edge_t{ targetTo, sourceFrom });
}

void FrontierShiftingData::findCutVertices()
{
  // frontiershifting is not allowed on cut vertices as removing them would
  // disconnect the chain. Therefore, identify cut vertices beforehand
  majorminer::identifiyCutVertices(m_cutVertices, m_victimSubgraph, m_nbNodes);
}

void FrontierShiftingData::setNbNodes(fuint32_t nbNodes)
{
  m_nbNodes = nbNodes;
}

bool FrontierShiftingData::isNowACutVertex(fuint32_t contested) const
{
  return majorminer::isCutVertex(m_victimSubgraph, contested, m_nbNodes);
}

void FrontierShiftingData::setVictimSource(fuint32_t node)
{
  m_victimSourceNode = node;
}

