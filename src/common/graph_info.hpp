#ifndef __MAJORMINER_GRAPH_INFO_HPP_
#define __MAJORMINER_GRAPH_INFO_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  struct ChimeraGraphInfo
  {
    ChimeraGraphInfo(){}
    ChimeraGraphInfo(fuint32_t x, fuint32_t y)
      : m_x(x), m_y(y) {}

    fuint32_t getXCoord(vertex_t vertex) const;
    fuint32_t getYCoord(vertex_t vertex) const;
    fuint32_t getWidth() const { return m_x; }
    fuint32_t getHeight() const { return m_y; }

    fuint32_t m_x;
    fuint32_t m_y;
  };

}




#endif