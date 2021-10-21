#include "qubo_modelling.hpp"

using namespace majorminer;



void QPolynomial::updateTerm(QVariable* varI, QVariable* varJ, qcoeff_t coeff)
{
  m_terms[orderedPair(varI, varJ)] += coeff;
}

graph_t QPolynomial::getConnectivityGraph() const
{
  graph_t connections{};
  for (const auto& term : m_terms)
  {
    const auto& vars = term.first;
    if (vars.first == nullptr || vars.second == nullptr || term.second == 0) continue;
    connections.insert(edge_t{vars.first->getIdx(), vars.second->getIdx()});
  }
  return connections;
}

QVariablePairMap QPolynomial::square() const
{
  for (const auto& term : m_terms)
  {
    if (term.first.second != nullptr) throw std::runtime_error("Squaring can only be done for linear polynomials!");
  }

  QVariablePairMap map{};
  for (auto it = m_terms.begin(); it != m_terms.end(); ++it)
  {
    for (auto it2 = it; it2 != m_terms.end(); ++it2)
    {
      if (&(*it) == &(*it2)) continue;
      map[orderedPair(it->first.first, it2->first.first)] += 2 * it->second + it2->second;
    }
  }
  return map;
}

QVariablePair QPolynomial::orderedPair(QVariable* varI, QVariable* varJ) const
{
  if (varI == varJ) return QVariablePair { varI, nullptr };
  else if (varI == nullptr) return QVariablePair{ varJ, varI };
  else if (varJ == nullptr) return QVariablePair{ varI, varJ };
  else if (varI->getIdx() < varJ->getIdx()) return QVariablePair{ varI, varJ };
  else return QVariablePair{ varJ, varI };
}


void QPolynomial::addTermMap(const QVariablePairMap& map, qcoeff_t multiplier)
{
  for (auto& term : map)
  {
    m_terms[term.first] += multiplier * term.second;
  }
}

void QPolynomial::addTerms(QVariableVec& vars, qcoeff_t coeff)
{
  for (auto var : vars) updateTerm(var, nullptr, coeff);
}

QVariableVec QModel::createBinaryVars(fuint32_t n)
{
  QVariableVec vars{};
  vars.resize(n);
  fuint32_t idx = m_variables.size();
  for (auto& var : vars)
  {
    var = new QVariable{ idx++ };
  }
  m_variables.insert(m_variables.end(), vars.begin(), vars.end());
  return vars;
}

QVariable* QModel::createBinaryVar()
{
  m_variables.push_back(new QVariable{ m_variables.size() });
  return m_variables.back();
}

QConstraint* QModel::createConstraint(QConstraintType type, qcoeff_t rhs, qcoeff_t penalty)
{
  m_constraints.push_back(new QConstraint{type, rhs, penalty});
  return m_constraints.back();
}

QModel::~QModel()
{
  for (auto cons : m_constraints) delete cons;
  for (auto var : m_variables) delete var;
}


QPolynomial QModel::reformulate() const
{
  QPolynomial qubo{};
  qubo += m_objective;
  for (auto cons : m_constraints)
  {
    qubo += *cons;
  }
  return qubo;
}

graph_t QModel::operator()() const
{
  auto poly = reformulate();
  return poly.getConnectivityGraph();
}