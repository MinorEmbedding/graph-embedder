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
    auto it2 = it;
    ++it2;
    for (; it2 != m_terms.end(); ++it2)
    {
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


QPolynomial QModel::reformulate()
{
  QPolynomial qubo{};
  qubo += m_objective;
  for (auto cons : m_constraints)
  {
    reformulateConstraint(*cons, qubo);
  }
  for (auto cons : m_internalConstraints)
  {
    reformulateConstraint(*cons, qubo);
  }
  return qubo;
}

graph_t QModel::operator()()
{
  auto poly = reformulate();
  return poly.getConnectivityGraph();
}

void QModel::reformulateConstraint(QConstraint& constraint, QPolynomial& poly)
{
  auto terms = constraint.getPolynomial().getTermMap();
  fuint32_t nbOnes = 0;
  fuint32_t nbMinusOnes = 0;
  fuint32_t otherTerms = 0;
  for (const auto& term : terms)
  {
    if (term.second == 0) continue;
    if (term.second == 1) nbOnes++;
    else if (term.second == -1) nbMinusOnes++;
    else otherTerms++;
  }
  if (otherTerms > 0)
  {
    poly += constraint;
  }
  else if (nbOnes > 8 && nbMinusOnes == 0 && constraint.getRhs() == 1.0)
  {
    switch (constraint.getType())
    {
      case QConstraintType::EQUAL:
        reformulateAlternative(constraint, poly, AlternativeConstraintType::EQUAL_ONE, nbOnes);
        break;
      case QConstraintType::LOWER_EQUAL:
        reformulateAlternative(constraint, poly, AlternativeConstraintType::LEQ_ONE, nbOnes);
        break;
      case QConstraintType::GREATER_EQUAL:
        reformulateAlternative(constraint, poly, AlternativeConstraintType::GEQ_ONE, nbOnes);
        break;
    }
  }
  else if (nbOnes > 8 && nbMinusOnes == 1)
  {
    reformulateAlternative(constraint, poly, AlternativeConstraintType::ABSORPTION, nbOnes);
  }
  else
  {
    poly += constraint;
  }


}


void QModel::reformulateAlternative(QConstraint& constraint, QPolynomial& poly, AlternativeConstraintType type, fuint32_t n)
{
  QVariableVec nodes{};
  nodes.reserve(n);
  auto penalty = constraint.getPenalty();

  const auto& terms = constraint.getPolynomial().getTermMap();
  QVariable* absorber = nullptr; // only if absorber constraint
  for (const auto& term : terms)
  {
    if (term.second == 0.0 || term.first.first == nullptr) continue;
    if (term.second == -1.0) absorber = term.first.first;
    else if (term.second == 1.0) nodes.push_back(term.first.first);
  }

  fuint32_t level = 0;
  bool lastBad = false;
  QVariable* layer0Bad = nullptr; // for GEQ_ONE a "bad" node in the lowest layer must be negated into the rosenberg polynomial
  while(nodes.size() > 2)
  {
    fuint32_t writeIdx = 0;
    for (fuint32_t readIdx = 0; readIdx < nodes.size(); readIdx += 2)
    {
      if (readIdx + 2 == nodes.size() && lastBad)
      { // last level a node was left alone
        lastBad = false;
        if (type != AlternativeConstraintType::GEQ_ONE)
        { // x1 + x2 - y = 0
          nodes[writeIdx++] = smallAbsorptionConstraint(nodes[readIdx], nodes[readIdx + 1], penalty);
        }
        else
        { // GEQ_ONE needs logical AND but if the current "bad" node is from layer 0, we have to negate the variable
          if (layer0Bad == nodes[readIdx + 1]) nodes[writeIdx++] = partialNegatedRosenbergPolynomial(poly, nodes[readIdx], nodes[readIdx + 1], penalty);
          else nodes[writeIdx++] = normalRosenbergPolynomial(poly, nodes[readIdx], nodes[readIdx + 1], penalty);
          layer0Bad = nullptr;
        }
      }
      else if (readIdx + 1 < nodes.size())
      { // condense simple
        if (type != AlternativeConstraintType::GEQ_ONE)
        { // x1 + x2 - y = 0
          nodes[writeIdx++] = smallAbsorptionConstraint(nodes[readIdx], nodes[readIdx + 1], penalty);
        }
        else
        { // for level=0: y = not(x1) * not(x2), enforce this by using Rosenberg polynomial (with negated x1 and x2)
          // else: y = x1 * x2 (logical and), again Rosenberg polynomial
          if (level == 0) nodes[writeIdx++] = negatedRosenbergPolynomial(poly, nodes[readIdx], nodes[readIdx + 1], penalty);
          else nodes[writeIdx++] = normalRosenbergPolynomial(poly, nodes[readIdx], nodes[readIdx + 1], penalty);
        }
      }
      else
      { // node left over
        if (level == 0) layer0Bad = nodes[readIdx];
        nodes[writeIdx++] = nodes[readIdx];
        lastBad = true;
      }
    }
    nodes.resize(writeIdx); // condensed a lot of nodes
    level++;
  }

  if (nodes.size() == 2)
  {
    switch(type)
    {
      case AlternativeConstraintType::EQUAL_ONE:
      { // insert constraint x1 + x2 = 1
        m_internalConstraints.push_back(new QConstraint{QConstraintType::EQUAL, 1, penalty});
        auto& cons = *m_internalConstraints.back();
        cons.addTerm(*nodes[0], 1.0);
        cons.addTerm(*nodes[1], 1.0);
        break;
      }
      case AlternativeConstraintType::LEQ_ONE:
      { // insert polynomial x1 * x2
        poly.addTerm(*nodes[0], *nodes[1], penalty);
        break;
      }
      case AlternativeConstraintType::GEQ_ONE:
      { // insert high penalty for either x1*x2 or (if layer0Bad == x2) x1 * (not(x2)) = x1 - x1x2
        if (layer0Bad != nodes[1])
        {
          poly.addTerm(*nodes[0], *nodes[1], -1.0);
          poly.addTerm(*nodes[0], 1.0);
        }
        break;
      }
      case AlternativeConstraintType::ABSORPTION:
      { // insert constraint x1 + x2 - absorber = 0
        smallAbsorptionConstraint(nodes[0], nodes[1], penalty, absorber);
        break;
      }
    }
  }

}

QVariable* QModel::normalRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty)
{ // y = x1 * x2
  return addRosenbergPolynomial<false, false>(poly, x1, x2, penalty);
}

QVariable* QModel::negatedRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty)
{ // y = not(x1) * not(x2)
  return addRosenbergPolynomial<true, true>(poly, x1, x2, penalty);
}

QVariable* QModel::partialNegatedRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty)
{ // y = x1 * not(x2)
  return addRosenbergPolynomial<false, true>(poly, x1, x2, penalty);
}


QVariable* QModel::smallAbsorptionConstraint(QVariable* x1, QVariable* x2, qcoeff_t penalty, QVariable* y)
{ // x1 + x2 - y = 0;
  if (y == nullptr) y = createBinaryVar();
  m_internalConstraints.push_back(new QConstraint{QConstraintType::EQUAL, 0, penalty});
  auto& cons = *m_internalConstraints.back();
  cons.addTerm(*x1, 1.0);
  cons.addTerm(*x2, 1.0);
  cons.addTerm(*y, -1.0);
  return y;
}

