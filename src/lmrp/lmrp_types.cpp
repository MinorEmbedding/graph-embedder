#include "lmrp_types.hpp"

using namespace majorminer;

ConnectedList::ConnectedList(vertex_t source, fuint32_t idx,
    fuint32_t nbMapped)
  : m_source(source), m_idx(idx),
    m_nbMapped(nbMapped), m_satisfied(false) {}

DijkstraVertex::DijkstraVertex(vertex_t target)
  : m_target(target) { reset(); }

DijkstraVertex::DijkstraVertex(vertex_t target, vertex_t parent,
  fuint32_t overlapCnt, fuint32_t nonOverlapCnt)
  : m_target(target), m_parent(parent),
    m_overlapCnt(overlapCnt), m_nonOverlapCnt(nonOverlapCnt),
    m_visited(false) {}

void DijkstraVertex::reset()
{
  m_parent = FUINT32_UNDEF;
  m_overlapCnt = FUINT32_UNDEF;
  m_nonOverlapCnt = FUINT32_UNDEF;
  m_visited = false;
}

bool DijkstraVertex::lowerTo(fuint32_t parent, fuint32_t overlap,
  fuint32_t nonOverlap)
{
  if (overlap < m_overlapCnt || (overlap == m_overlapCnt
    && nonOverlap < m_nonOverlapCnt))
  {
    std::cout << "Changing parent from " << m_parent << " to " << parent << std::endl;
    m_parent = parent;
    m_overlapCnt = overlap;
    m_nonOverlapCnt = nonOverlap;
    return true;
  }
  return false;
}

bool DijkstraVertex::visited()
{
  bool original = m_visited;
  m_visited = true;
  return original;
}

