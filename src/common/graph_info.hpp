#ifndef __MAJORMINER_GRAPH_INFO_HPP_
#define __MAJORMINER_GRAPH_INFO_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  struct ChimeraGraphInfo
  {
    ChimeraGraphInfo(): m_width(0), m_height(0) {}
    ChimeraGraphInfo(fuint32_t width, fuint32_t height)
      : m_width(width), m_height(height) {}

    fuint32_t getXCoord(vertex_t vertex) const;
    fuint32_t getYCoord(vertex_t vertex) const;
    fuint32_t getWidth() const { return m_width; }
    fuint32_t getHeight() const { return m_height; }

    fuint32_t m_width;
    fuint32_t m_height;
  };

  struct KingGraphInfo
  {
    KingGraphInfo(): m_width(0), m_height(0) {}
    KingGraphInfo(fuint32_t width, fuint32_t height)
      : m_width(width), m_height(height) {}

    fuint32_t getXCoord(vertex_t vertex) const { return vertex % m_width; }
    fuint32_t getYCoord(vertex_t vertex) const { return vertex / m_width; }
    fuint32_t getWidth() const { return m_width; }
    fuint32_t getHeight() const { return m_height; }

    fuint32_t m_width;
    fuint32_t m_height;
  };
}




#endif