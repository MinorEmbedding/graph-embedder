#include "embedding_visualizer.hpp"

using namespace majorminer;

const static std::string colors[] = {
  "Lightgray","Wheat","Paleturquoise","Palegoldenrod","Powderblue","Thistle","Lightblue","Palegreen","Lightsteelblue","Aquamarine","Khaki","Lightgreen","Lightskyblue","Plum","Silver","Lightsalmon","Skyblue","Lightcoral","Violet","Hotpink","Salmon",
  "Thistle","Lightblue","Palegreen","Lightsteelblue","Aquamarine","Khaki","Lightgreen","Lightskyblue","Plum","Silver","Lightsalmon","Skyblue","Lightcoral","Violet","Hotpink","Salmon","Burlywood","Darksalmon","Tan","Mediumslateblue","Sandybrown",
  "Khaki","Lightgreen","Lightskyblue","Plum","Silver","Lightsalmon","Skyblue","Lightcoral","Violet","Hotpink","Salmon","Burlywood","Darksalmon","Tan","Mediumslateblue","Sandybrown","Coral","Cornflowerblue","Darkgray","Darkseagreen","Mediumpurple",
  "Lightsalmon","Skyblue","Lightcoral","Violet","Hotpink","Salmon","Burlywood","Darksalmon","Tan","Mediumslateblue","Sandybrown","Coral","Cornflowerblue","Darkgray","Darkseagreen","Mediumpurple","Orchid","Palevioletred","Rosybrown","Tomato","Mediumaquamarine",
  "Salmon","Burlywood","Darksalmon","Tan","Mediumslateblue","Sandybrown","Coral","Cornflowerblue","Darkgray","Darkseagreen","Mediumpurple","Orchid","Palevioletred","Rosybrown","Tomato","Mediumaquamarine","Greenyellow","Darkkhaki","Indianred","Mediumorchid","Slateblue",
  "Sandybrown","Coral","Cornflowerblue","Darkgray","Darkseagreen","Mediumpurple","Orchid","Palevioletred","Rosybrown","Tomato","Mediumaquamarine","Greenyellow","Darkkhaki","Indianred","Mediumorchid","Slateblue","Royalblue","Dodgerblue","Turquoise","Mediumturquoise","Deeppink",
  "Mediumpurple","Orchid","Palevioletred","Rosybrown","Tomato","Mediumaquamarine","Greenyellow","Darkkhaki","Indianred","Mediumorchid","Slateblue","Royalblue","Dodgerblue","Turquoise","Mediumturquoise","Deeppink","Blueviolet","Lightslategray","Peru","Aqua","Blue",
  "Mediumaquamarine","Greenyellow","Darkkhaki","Indianred","Mediumorchid","Slateblue","Royalblue","Dodgerblue","Turquoise","Mediumturquoise","Deeppink","Blueviolet","Lightslategray","Peru","Aqua","Blue","Cadetblue","Chartreuse","Darkorange","Darkorchid","Deepskyblue",
  "Slateblue","Royalblue","Dodgerblue","Turquoise","Mediumturquoise","Deeppink","Blueviolet","Lightslategray","Peru","Aqua","Blue","Cadetblue","Chartreuse","Darkorange","Darkorchid","Deepskyblue","Fuchsia","Gold","Gray","Lime","Limegreen",
  "Deeppink","Blueviolet","Lightslategray","Peru","Aqua","Blue","Cadetblue","Chartreuse","Darkorange","Darkorchid","Deepskyblue","Fuchsia","Gold","Gray","Lime","Limegreen","Orange","Orangered","Red","Slategray","Springgreen",
  "Blue","Cadetblue","Chartreuse","Darkorange","Darkorchid","Deepskyblue","Fuchsia","Gold","Gray","Lime","Limegreen","Orange","Orangered","Red","Slategray","Springgreen","Yellow","Yellowgreen","Goldenrod","Lawngreen","Mediumspringgreen",
  "Deepskyblue","Fuchsia","Gold","Gray","Lime","Limegreen","Orange","Orangered","Red","Slategray","Springgreen","Yellow","Yellowgreen","Goldenrod","Lawngreen","Mediumspringgreen","Steelblue","Chocolate","Crimson","Mediumseagreen","Mediumvioletred",
  "Limegreen","Orange","Orangered","Red","Slategray","Springgreen","Yellow","Yellowgreen","Goldenrod","Lawngreen","Mediumspringgreen","Steelblue","Chocolate","Crimson","Mediumseagreen","Mediumvioletred","Firebrick","Brown","Darkturquoise","Darkviolet","Dimgray",
  "Springgreen","Yellow","Yellowgreen","Goldenrod","Lawngreen","Mediumspringgreen","Steelblue","Chocolate","Crimson","Mediumseagreen","Mediumvioletred","Firebrick","Brown","Darkturquoise","Darkviolet","Dimgray","Lightseagreen","Mediumblue","Sienna","Darkslateblue","Darkgoldenrod",
  "Mediumspringgreen","Steelblue","Chocolate","Crimson","Mediumseagreen","Mediumvioletred","Firebrick","Brown","Darkturquoise","Darkviolet","Dimgray","Lightseagreen","Mediumblue","Sienna","Darkslateblue","Darkgoldenrod","Seagreen","Olivedrab","Forestgreen","Saddlebrown","Darkolivegreen",
  "Mediumvioletred","Firebrick","Brown","Darkturquoise","Darkviolet","Dimgray","Lightseagreen","Mediumblue","Sienna","Darkslateblue","Darkgoldenrod","Seagreen","Olivedrab","Forestgreen","Saddlebrown","Darkolivegreen","Darkblue","Darkcyan","Darkmagenta","Darkred","Midnightblue",
  "Dimgray","Lightseagreen","Mediumblue","Sienna","Darkslateblue","Darkgoldenrod","Seagreen","Olivedrab","Forestgreen","Saddlebrown","Darkolivegreen","Darkblue","Darkcyan","Darkmagenta","Darkred","Midnightblue","Darkslategray","Green","Indigo","Maroon","Navy",
  "Darkgoldenrod","Seagreen","Olivedrab","Forestgreen","Saddlebrown","Darkolivegreen","Darkblue","Darkcyan","Darkmagenta","Darkred","Midnightblue","Darkslategray","Green","Indigo","Maroon","Navy","Olive","Purple","Teal","Darkgreen",
  "Darkolivegreen","Darkblue","Darkcyan","Darkmagenta","Darkred","Midnightblue","Darkslategray","Green","Indigo","Maroon","Navy","Olive","Purple","Teal","Darkgreen",
  "Midnightblue","Darkslategray","Green","Indigo","Maroon","Navy","Olive","Purple","Teal","Darkgreen",
  "Navy","Olive","Purple","Teal","Darkgreen"
};


void EmbeddingVisualizer::draw(const embedding_mapping_t& embedding)
{
  if (!m_initialized) initialize();

  m_embedding = &embedding;
  auto width = getWidth();
  auto height = getHeight();
  m_svg << "<svg height=\"" << height
     << "\" width=\"" << width << "\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
     << width << " " << height << "\">";

  std::string color = "black";
  for (const auto& edge : m_target)
  {
    drawEdge(edge, color);
  }

  drawChains();
  drawInterChainConnections();

  double radius = getRadius();
  for (auto node : m_nodes)
  {
    drawNode(node.first, radius, node.second);
  }

  drawNodes();


  m_svg << "</svg>";
  writeToFile();
}

const std::string& EmbeddingVisualizer::getColor(fuint32_t node)
{
  return colors[node % (sizeof(colors))];
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
    std::cout << groupIt->first << " -> " << std::distance(rangeIt.first, rangeIt.second) << std::endl ;
    for (auto targetNodeIt = rangeIt.first; targetNodeIt != rangeIt.second; ++targetNodeIt)
    {
      auto innerIt = targetNodeIt;
      ++innerIt;
      for(; innerIt != rangeIt.second; ++innerIt)
      {
        if (m_target.contains(edge_t{targetNodeIt->second, innerIt->second}))
        { // connection within a chain, draw it
          std::cout << "CHAIN" << std::endl;
          drawEdge(edge_t{targetNodeIt->second, innerIt->second}, color, getStrokeWidth());
        }
        else if (m_target.contains(edge_t{innerIt->second, targetNodeIt->second}))
        { // connection within a chain, draw it
          std::cout << "CHAIN" << std::endl;

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
    drawNode(mapped.second, radius + sizeDelta * it->second, m_nodes[mapped.second], true);
    it->second--;
  }
}

void EmbeddingVisualizer::drawNode(fuint32_t node, double radius, const Coordinate_t& coordinate, bool fill)
{
  m_svg << "<circle id=\"node_" << node << "\" r=\"" << radius << "\" cx=\"" << coordinate.first
         << "\" cy=\"" << coordinate.second << "\" fill=\""
         << (fill ? m_sourceNodeColors[node] : "none") << "\" stroke=\"black\" />";
}

void EmbeddingVisualizer::drawEdge(const edge_t& edge, const std::string& color, double stroke)
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

