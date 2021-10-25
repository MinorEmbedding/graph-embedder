#ifndef __MAJORMINER_QUBO_PROBLEMS_HPP_
#define __MAJORMINER_QUBO_PROBLEMS_HPP_

#include "qubo_modelling.hpp"

namespace majorminer
{

  template<typename Functor>
  graph_t quboTSP(size_t n, Functor distanceFunc)
  {
    qcoeff_t penalty = 1000;
    QModel model{};
    std::vector<QVariableVec> cities{};
    cities.reserve(n);
    for (fuint32_t i = 0; i < n; ++i)
    {
      cities.push_back(model.createBinaryVars(n));
      auto& cons = *model.createConstraint(QConstraintType::EQUAL, 1, penalty);
      cons.addTerms(cities.back(), 1.0);
    }
    for (fuint32_t i = 0; i < n; ++i)
    {
      auto& cons = *model.createConstraint(QConstraintType::EQUAL, 1, penalty);
      for (fuint32_t j = 0; j < n; ++j)
      {
        cons.addTerm(*cities[j][i], 1.0);
      }
    }

    auto& obj = model.getObjective();
    for (fuint32_t time = 0; time < n; ++time)
    {
      fuint32_t nextTime = (time + 1) % n;
      for (fuint32_t i = 0; i < n; ++i)
      {
        for (fuint32_t j = 0; j < n; ++j)
        {
          if (i == j) continue;
          obj.addTerm(*cities[i][time], *cities[j][nextTime], distanceFunc(i,j));
        }
      }
    }
    return model();
  }

}


#endif