#include "embedding_visualizer.hpp"

using namespace majorminer;


void EmbeddingVisualizer::draw(const embedding_mapping_t& embedding)
{
  if (!m_initialized) initialize();

  m_embedding = &embedding;
  auto width = getWidth();
  auto height = getHeight();
  m_svg << "<svg height=\"" << height
     << "\" width=\"" << width << "\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
     << width << " " << height << "\">";

  for (const auto& edge : m_target)
  {
    drawEdge(edge);
  }

  for (auto node : m_nodes)
  {
    drawNode(node.first, node.second);
  }


  m_svg << "</svg>";
  writeToFile();
}

void EmbeddingVisualizer::initialize()
{
  for (const auto& edge : m_target)
  {
    // initialize node positions
    if (!m_nodes.contains(edge.first))
    {
      m_nodes.insert(std::make_pair(edge.first, insertNode(edge.first)));
    }
    if (!m_nodes.contains(edge.second))
    {
      m_nodes.insert(std::make_pair(edge.second, insertNode(edge.second)));
    }

    // initialize edge midpoints
    fuint32_t size = m_edgeSamples.size();
    fuint32_t nbSamples = insertEdge(m_edgeSamples, edge);
    m_edgePtrs.insert(std::make_pair(edge, std::make_pair(size, nbSamples)));
  }
  m_initialized = true;
}


void EmbeddingVisualizer::writeToFile()
{
  std::stringstream name;
  name << m_filename << "_" << (++m_iteration) << ".svg";
  std::ofstream f(name.str());
  if (!f.is_open()) throw std::runtime_error("Could not open file for visualization of embedding.");

  f << m_svg.str();

  f.close();
  m_svg.str(std::string());
}

void EmbeddingVisualizer::drawNode(fuint32_t node, const Coordinate_t& coordinate)
{
  double radius = getRadius();
  m_svg << "<circle id=\"node_" << node << "\" r=\"" << radius << "\" cx=\"" << coordinate.first
         << "\" cy=\"" << coordinate.second << "\" fill=\"black\" />";
}

void EmbeddingVisualizer::drawEdge(const edge_t& edge)
{
  auto edgePtr = m_edgePtrs[edge];
  const auto& startPos = m_nodes[edge.first];
  const auto& endPos = m_nodes[edge.second];
  #define COORD(c) c.first << "," << c.second << " "
  m_svg << "<polyline points=\"" <<  COORD(startPos);
  for (fuint32_t i = edgePtr.first; i < (edgePtr.first + edgePtr.second); ++i)
  {
    const auto& coordinate = m_edgeSamples[i];
    m_svg << COORD(coordinate);
  }
  m_svg << COORD(endPos) << "\" fill=\"none\" stroke=\"black\"/>";
}


#define X_CELL 6
#define Y_CELL 9

fuint32_t ChimeraVisualizer::insertEdge(Vector<Coordinate_t>& coords, const edge_t& edge)
{
  const auto& n1Coord = m_nodes[edge.first];
  const auto& n2Coord = m_nodes[edge.second];
  if (n1Coord.first != n2Coord.first && n1Coord.second != n2Coord.second)
  { // within cell
    return 0;
  }
  else if (n1Coord.first != n2Coord.first)
  { // within row
    double withinCellOffset = getNodeSize() * (X_CELL-1);
    if ((n1Coord.first + withinCellOffset) > n2Coord.first) return 0;
    coords.push_back(Coordinate_t{ (n1Coord.first + n2Coord.first) / 2.0, n1Coord.second + getNodeSize() });
    return 1;
  }
  else if (n1Coord.second != n2Coord.second)
  { // within column
    coords.push_back(Coordinate_t{ n1Coord.first - getNodeSize(), (n1Coord.second + n2Coord.second) / 2.0 });
    return 1;
  }
  return 0;
}

Coordinate_t ChimeraVisualizer::insertNode(fuint32_t v) const
{
  auto nodeSize = getNodeSize();
  fuint32_t row = v / m_nbVerticesPerRow;
  fuint32_t col = (v - (row * m_nbVerticesPerRow)) / 8;
  bool left = (v % 8) < 4;
  fuint32_t cell_row = v % 4;
  return std::make_pair(
      (1 + X_CELL * col  + (!left ? 4 : 1 )) * nodeSize,
      ((1 + row * Y_CELL + 2 * cell_row) * nodeSize)
  );
}

double ChimeraVisualizer::getWidth() const
{
  return m_nbCols * X_CELL * getNodeSize();
}

double ChimeraVisualizer::getHeight() const
{
  return m_nbRows * Y_CELL * getNodeSize();
}

