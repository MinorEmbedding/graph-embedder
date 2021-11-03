#include <filesystem>
#include "utils/test_common.hpp"
#include "utils/qubo_problems.hpp"

using namespace majorminer;
namespace fs = std::filesystem;

#define PENALTY 10000

namespace
{
  void createMatrixFile(QConstraintType type, size_t n, std::string tName, std::string folder)
  {
    fs::create_directories(folder);
    std::stringstream ss;
    ss  << folder << "consmatrix_" << tName << "_" << n << ".txt";
    std::string filename = ss.str();
    auto model = createConstraintMatrix(n, type, PENALTY);
    auto reformulated = model.reformulate();
    dumpModel(filename, reformulated);
  }
}

TEST(QuboProblem, Simple_TSP_10)
{
  auto graph = majorminer::quboTSP(7, [](fuint32_t i, fuint32_t j){ return i+j; });
  containsEdges(graph, {{0, 7}, {1, 8}, {0, 1}, {0, 2}});
}

TEST(QuboProblem, DISABLED_Matrix_EQ)
{
  for (fuint32_t i = 8; i <= 20; ++i)
  {
    createMatrixFile(QConstraintType::EQUAL, i, "EQ", "consmatrices/EQ/");
  }
}

TEST(QuboProblem, DISABLED_Matrix_LEQ)
{
  for (fuint32_t i = 8; i <= 20; ++i)
  {
    createMatrixFile(QConstraintType::LOWER_EQUAL, i, "LEQ", "consmatrices/LEQ/");
  }
}

TEST(QuboProblem, DISABLED_Matrix_GEQ)
{
  for (fuint32_t i = 8; i <= 20; ++i)
  {
    createMatrixFile(QConstraintType::GREATER_EQUAL, i, "GEQ", "consmatrices/GEQ/");
  }
}