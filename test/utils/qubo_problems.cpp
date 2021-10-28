#include "qubo_problems.hpp"

using namespace majorminer;


QModel majorminer::createConstraintMatrix(fuint32_t n, QConstraintType type, qcoeff_t penalty)
{
  QModel model{};
  Vector<QVariableVec> matrix{};
  matrix.reserve(n);

  for (fuint32_t i = 0; i < n; ++i)
  {
    matrix.push_back(model.createBinaryVars(n));
    auto& cons = *model.createConstraint(type, 1.0, penalty);
    cons.addTerms(matrix.back());
  }
  for (fuint32_t j = 0; j < n; ++j)
  {
    auto& cons = *model.createConstraint(type, 1.0, penalty);
    for (fuint32_t i = 0; i < n; ++i)
    {
      cons.addTerm(*matrix[i][j]);
    }
  }
  return model;
}
