#include "embedding_visualizer.hpp"

using namespace majorminer;
namespace fs = std::filesystem;

const static std::string colors[] = {
  "Cyan", "purple", "green", "blue", "yellow", "magenta", "Navy","Aquamarine","Lawngreen","Violet","Darkslateblue","Darkblue","Sienna","Crimson","Tomato","Orangered","Mediumorchid","Mediumslateblue","Rosybrown","Skyblue","Cadetblue","Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue",
  "Darkblue","Sienna","Crimson","Tomato","Orangered","Mediumorchid","Mediumslateblue","Rosybrown","Skyblue","Cadetblue","Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet",
  "Mediumorchid","Mediumslateblue","Rosybrown","Skyblue","Cadetblue","Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange",
  "Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen",
  "Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink",
  "Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan",
  "Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow",
  "Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow","Khaki","Darkgreen","Dimgray","Mediumaquamarine","Cornflowerblue",
  "Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow","Khaki","Darkgreen","Dimgray","Mediumaquamarine","Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray",
  "Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow","Khaki","Darkgreen","Dimgray","Mediumaquamarine","Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue",
  "Yellow","Khaki","Dimgray","Mediumaquamarine","Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru",
  "Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta",
  "Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon",
  "Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen",
  "Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue",
  "Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green",
  "Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown",
  "Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown","Fuchsia","Purple","Olive","Orchid",
  "Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown","Fuchsia","Purple","Olive","Orchid",
  "Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown","Fuchsia","Purple","Olive","Orchid",
  "Sandybrown","Fuchsia","Purple","Olive","Orchid"
};
const static fuint32_t nbColors = sizeof(colors)/sizeof(colors[0]);

#define Y_OFFSET 100

void EmbeddingVisualizer::setupDrawing(const embedding_mapping_t& embedding)
{
  if (!m_initialized) initialize();

  m_embedding = &embedding;
  m_svg << m_prepared;

  double fontSize = getWidth() / 30;

  std::stringstream titleText {};
  titleText << "Iteration " << (m_iteration + 1) << ": ";
  m_svg << "<text x=\"" << getRadius() << "\" y=\"50\" font-size=\"" << fontSize << "\">" << titleText.str();
}

void EmbeddingVisualizer::draw(const embedding_mapping_t& embedding, const char* title)
{
  setupDrawing(embedding);
  if (title != nullptr) m_svg << title;
  finishDrawing();
}

void EmbeddingVisualizer::finishDrawing()
{
  m_svg << "</text>";
  drawInterChainConnections();
  drawChains();
  drawNodes();

  m_svg << "</svg>";
  writeToFile();
  m_embedding = nullptr;
}

const std::string& EmbeddingVisualizer::getColor(fuint32_t node)
{
  return colors[node % nbColors];
}


// inefficient but who cares?
void EmbeddingVisualizer::drawInterChainConnections()
{
  UnorderedMultiMap<fuint32_t, fuint32_t> reverseMapping{};
  for (const auto& mapped : *m_embedding)
  {
    reverseMapping.insert(std::make_pair(mapped.second, mapped.first));
  }
  std::string color = "red";
  for (const auto& edge : m_target)
  {
    auto uRev = reverseMapping.equal_range(edge.first);
    if (uRev.first == uRev.second) continue;
    auto vRev = reverseMapping.equal_range(edge.second);
    if (vRev.first == vRev.second) continue;
    bool found = false;
    for (auto u = uRev.first; u != uRev.second && !found; ++u)
    {
      for (auto v = vRev.first; v != vRev.second && !found; ++v)
      { // does m_source contain a (u,v) or a (v,u) edge?
        if (m_source.contains(edge_t {u->second, v->second}))
        { drawEdge(edge, color, 3); found = true; }
        else if (m_source.contains(edge_t {v->second, u->second}))
        { drawEdge(edge, color, 3); found = true; }

      }
    }
  }
}

void EmbeddingVisualizer::initialize()
{
  fs::path path = m_filename;
  auto parent = path.parent_path();
  fs::create_directories(parent);
  for (const auto& edge : m_target)
  {
    // initialize node positions
    if (!m_nodes.contains(edge.first))
    {
      m_nodes.insert(std::make_pair(edge.first, insertNode(edge.first)));
      m_sourceNodeColors.insert(std::make_pair(edge.first, getColor(edge.first)));
    }
    if (!m_nodes.contains(edge.second))
    {
      m_nodes.insert(std::make_pair(edge.second, insertNode(edge.second)));
      m_sourceNodeColors.insert(std::make_pair(edge.second, getColor(edge.second)));
    }

    // initialize edge midpoints
    fuint32_t size = m_edgeSamples.size();
    fuint32_t nbSamples = insertEdge(m_edgeSamples, edge);
    m_edgePtrs.insert(std::make_pair(edge, std::make_pair(size, nbSamples)));
  }

  auto width = getWidth();
  auto height = getHeight() + Y_OFFSET;
  m_svg << "<svg height=\"" << height
     << "\" width=\"" << width << "\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
     << width << " " << height << "\">"
     << "<rect x=\"0\" y=\"0\" width=\"" << width
     << "\" height=\"" << height << "\" fill=\"white\"/>";

  std::string color = "black";
  for (const auto& edge : m_target)
  {
    drawEdge(edge, color);
  }

  double radius = getRadius();
  std::string none = "white";
  for (auto node : m_nodes)
  {
    drawNode(node.first, radius, node.second, none);
  }
  m_prepared = m_svg.str();
  m_svg.str(std::string());

  m_initialized = true;
}

void EmbeddingVisualizer::drawChains()
{
  if (m_embedding->size() == 0) return;
  auto groupIt = m_embedding->begin();

  while(groupIt != m_embedding->end())
  {
    auto rangeIt = m_embedding->equal_range(groupIt->first);
    const auto& color = m_sourceNodeColors[groupIt->first];
    for (auto targetNodeIt = rangeIt.first; targetNodeIt != rangeIt.second; ++targetNodeIt)
    {
      auto innerIt = targetNodeIt;
      ++innerIt;
      for(; innerIt != rangeIt.second; ++innerIt)
      {
        if (m_target.contains(edge_t{targetNodeIt->second, innerIt->second}))
        { // connection within a chain, draw it
          drawEdge(edge_t{targetNodeIt->second, innerIt->second}, color, getStrokeWidth());
        }
        else if (m_target.contains(edge_t{innerIt->second, targetNodeIt->second}))
        { // connection within a chain, draw it
          drawEdge(edge_t{innerIt->second, targetNodeIt->second}, color, getStrokeWidth());
        }
      }
    }
    groupIt = rangeIt.second;
  }
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

void EmbeddingVisualizer::drawNodes()
{
  UnorderedMap<fuint32_t, fuint32_t> targetNodesUsed{};
  fuint32_t maxVal = 0;
  for (const auto& mapped : *m_embedding)
  {
    targetNodesUsed[mapped.second]++;
    maxVal = std::max(maxVal, targetNodesUsed[mapped.second]);
  }

  if (maxVal == 0) return;

  double sizeDelta = (getRadius() / (maxVal * 2));

  double radius = getRadius();

  for (const auto& mapped : *m_embedding)
  {
    auto it = targetNodesUsed.find(mapped.second);
    drawNode(mapped.second, radius + sizeDelta * (--it->second), m_nodes[mapped.second], m_sourceNodeColors[mapped.first], it->second + 2);
  }
}

void EmbeddingVisualizer::drawNode(fuint32_t node, double radius, const Coordinate_t& coordinate, const std::string& color, fuint32_t n)
{
  m_svg << "<circle id=\"node_" << node << "_" << n << "\" r=\"" << radius << "\" cx=\"" << coordinate.first
         << "\" cy=\"" << (coordinate.second + Y_OFFSET) << "\" fill=\""
         << color << "\" stroke=\"black\" />";
}

void EmbeddingVisualizer::drawEdge(const edge_t& edge, const std::string& color, double stroke)
{
  auto edgePtr = m_edgePtrs[edge];
  const auto& startPos = m_nodes[edge.first];
  const auto& endPos = m_nodes[edge.second];
  #define COORD(c) c.first << "," << (c.second + Y_OFFSET) << " "
  m_svg << "<polyline points=\"" <<  COORD(startPos);
  for (fuint32_t i = edgePtr.first; i < (edgePtr.first + edgePtr.second); ++i)
  {
    const auto& coordinate = m_edgeSamples[i];
    m_svg << COORD(coordinate);
  }
  m_svg << COORD(endPos) << "\" fill=\"none\" stroke=\""
        << color << "\" stroke-width=\"" << stroke << "\"/>";
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

fuint32_t KingsVisualizer::insertEdge(Vector<Coordinate_t>& /* coords */, const edge_t& /* edge */)
{
  return 0;
}

Coordinate_t KingsVisualizer::insertNode(fuint32_t v) const
{
  double nodeSize = getNodeSize();
  fuint32_t row = v / m_nbCols;
  fuint32_t col = v % m_nbCols;
  return std::make_pair((1 + 2 * col) * nodeSize, (1 + 2 * row) * nodeSize);
}

double KingsVisualizer::getWidth() const
{
  return ((1 + m_nbCols * 2) * getNodeSize());
}

double KingsVisualizer::getHeight() const
{
  return ((1 + m_nbRows * 2) * getNodeSize());
}


fuint32_t GenericVisualizer::insertEdge(Vector<Coordinate_t>& /* coords */, const edge_t& /* edge */)
{
  return 0;
}

Coordinate_t GenericVisualizer::insertNode(fuint32_t v) const
{
  auto findIt = m_coordinates.find(v);
  if (findIt == m_coordinates.end()) throw std::runtime_error("Node not contained in coordinate map!");

  return Coordinate_t{ (1 + findIt->second.first) * getNodeSize(), (1 + findIt->second.second) * getNodeSize() };
}

double GenericVisualizer::getWidth() const
{
  return ((2 + m_width) * getNodeSize());
}

double GenericVisualizer::getHeight() const
{
  return ((2 + m_height) * getNodeSize());
}
