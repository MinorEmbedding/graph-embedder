#include "graph_gen.hpp"

using namespace majorminer;

graph_t majorminer::generate_chimera(fuint32_t rows, fuint32_t cols)
{
  graph_t graph{};
  fuint32_t currentNode = 0;
  fuint32_t lowerUnitCellOffset = cols * 8;
  for (fuint32_t row = 0; row < rows; ++row)
  {

    for (fuint32_t col = 0; col < cols; ++col)
    {
      fuint32_t iOffset = currentNode;
      fuint32_t jOffset = currentNode + 4;

      // add edges within unit cell
      for (fuint32_t i = 0; i < 4; ++i)
      {
        for (fuint32_t j = 0; j < 4; ++j)
        {
          graph.insert(std::make_pair(iOffset + i, jOffset + j));
        }
      }

      // add edges to right unit cell
      if (col + 1 != cols)
      {
        graph.insert(std::make_pair(jOffset, jOffset + 8));
        graph.insert(std::make_pair(jOffset + 1, jOffset + 9));
        graph.insert(std::make_pair(jOffset + 2, jOffset + 10));
        graph.insert(std::make_pair(jOffset + 3, jOffset + 11));
      }

      // add edges to unit cell below
      if (row + 1 != rows)
      {
        graph.insert(std::make_pair(iOffset, iOffset + lowerUnitCellOffset));
        graph.insert(std::make_pair(iOffset + 1, iOffset + lowerUnitCellOffset + 1));
        graph.insert(std::make_pair(iOffset + 2, iOffset + lowerUnitCellOffset + 2));
        graph.insert(std::make_pair(iOffset + 3, iOffset + lowerUnitCellOffset + 3));
      }

      currentNode += 8;
    }

  }

  return graph;
}


graph_t majorminer::generate_king(fuint32_t rows, fuint32_t cols)
{
  graph_t graph{};
  if (rows == 0 || cols == 0) return graph;
  fuint32_t currentNode = 0;
  fuint32_t lowerLeftOffset = cols - 1;
  fuint32_t lowerRightOffset = cols + 1;
  for (fuint32_t row = 0; row < rows; ++row)
  {
    for (fuint32_t col = 0; col < cols; ++col)
    {
      if (col + 1 != cols) graph.insert(std::make_pair(currentNode, currentNode + 1));
      if (row + 1 != rows)
      {
        graph.insert(std::make_pair(currentNode, currentNode + cols));
        if (col > 0) graph.insert(std::make_pair(currentNode, currentNode + lowerLeftOffset));
        if (col + 1 != cols) graph.insert(std::make_pair(currentNode, currentNode + lowerRightOffset));
      }
      currentNode++;
    }
  }

  return graph;
}