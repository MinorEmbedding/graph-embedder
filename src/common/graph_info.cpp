#include <common/graph_info.hpp>

using namespace majorminer;

fuint32_t ChimeraGraphInfo::getXCoord(vertex_t vertex) const
{
  return (vertex % (m_x * 8)) / 8;
}

fuint32_t ChimeraGraphInfo::getYCoord(vertex_t vertex) const
{
 return vertex / (m_x * 8);
}


