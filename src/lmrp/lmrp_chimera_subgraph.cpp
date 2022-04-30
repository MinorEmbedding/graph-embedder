#include "lmrp_chimera_subgraph.hpp"

using namespace majorminer;


bool ChimeraLMRPSubgraph::getSubgraph(vertex_t contained, nodeset_t& subgraph)
{
  fuint32_t x = m_graph.getXCoord(contained);
  fuint32_t y = m_graph.getYCoord(contained);
  m_updateLock.lock();
  bool success = checkOccupation(x, y, contained);
  if (success)
  {
    neighborhood(x,y,contained,
      [&](const fuint32_pair_t& pair, bool inCrater){
        if (inCrater) m_occupied.insert(pair);
    });
  }
  m_updateLock.unlock();
  if (success)
  {
    subgraph.clear();
    fuint32_t width = m_graph.getWidth();
    neighborhood(x,y,contained,
      [&](const fuint32_pair_t& pair, bool inCrater){
        if (inCrater) // pair := (x_coord, y_coord)
        {
          fuint32_t base = (width * pair.second + pair.first) * 8;
          for (fuint32_t i = 0; i < 8; ++i)
          {
            subgraph.insert(base + i);
          }
        }
    });
  }
  return success;
}

bool ChimeraLMRPSubgraph::isBeingDestroyed(vertex_t contained)
{
  fuint32_t x = m_graph.getXCoord(contained);
  fuint32_t y = m_graph.getYCoord(contained);

  m_updateLock.lock_shared();
  bool occupied = checkOccupation(x, y, contained);
  m_updateLock.unlock_shared();

  return occupied;
}

bool ChimeraLMRPSubgraph::checkOccupation(fuint32_t x, fuint32_t y, vertex_t contained) const
{
  bool occupied = false;
  neighborhood(x,y, contained,
    [&](const fuint32_pair_t& pair, bool){
      occupied |= m_occupied.contains(pair);
  });
  return occupied;
}

void ChimeraLMRPSubgraph::commit(vertex_t contained)
{
  fuint32_t x = m_graph.getXCoord(contained);
  fuint32_t y = m_graph.getYCoord(contained);
  m_updateLock.lock();
  neighborhood(x,y,contained,
    [&](const fuint32_pair_t& pair, bool inCrater){
      if (inCrater) m_occupied.unsafe_erase(pair);
    });
  m_updateLock.unlock();
}
