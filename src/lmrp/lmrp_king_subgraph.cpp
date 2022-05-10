#include <lmrp/lmrp_king_subgraph.hpp>

using namespace majorminer;


bool KingLMRPSubgraph::getSubgraph(vertex_t contained, nodeset_t& subgraph)
{
  fuint32_t x = m_graph.getXCoord(contained);
  fuint32_t y = m_graph.getYCoord(contained);
  m_updateLock.lock();
  bool success = !checkOccupation(x, y);
  if (success)
  {
    neighborhood(x,y,
      [&](const fuint32_pair_t& pair, bool inCrater){
        if (inCrater) m_occupied.insert(pair);
    });
  }
  m_updateLock.unlock();
  if (success)
  {
    subgraph.clear();
    fuint32_t width = m_graph.getWidth();
    neighborhood(x,y,
      [&](const fuint32_pair_t& pair, bool inCrater){
        if (inCrater) // pair := (x_coord, y_coord)
        {
          subgraph.insert(pair.first * width + pair.second);
        }
    });
  }
  return success;
}

bool KingLMRPSubgraph::isBeingDestroyed(vertex_t contained)
{
  fuint32_t x = m_graph.getXCoord(contained);
  fuint32_t y = m_graph.getYCoord(contained);

  m_updateLock.lock_shared();
  bool occupied = checkOccupation(x, y);
  m_updateLock.unlock_shared();

  return occupied;
}

bool KingLMRPSubgraph::checkOccupation(fuint32_t x, fuint32_t y) const
{
  bool occupied = false;
  neighborhood(x,y,
    [&](const fuint32_pair_t& pair, bool){
      occupied |= m_occupied.contains(pair);
  });
  return occupied;
}

void KingLMRPSubgraph::commit(vertex_t contained)
{
  fuint32_t x = m_graph.getXCoord(contained);
  fuint32_t y = m_graph.getYCoord(contained);
  m_updateLock.lock();
  neighborhood(x,y,
    [&](const fuint32_pair_t& pair, bool inCrater){
      if (inCrater) m_occupied.unsafe_erase(pair);
    });
  m_updateLock.unlock();
}


