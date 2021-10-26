#ifndef __MAJORMINER_QUBO_MODELLING_HPP_
#define __MAJORMINER_QUBO_MODELLING_HPP_

#include <vector>
#include <unordered_map>

#include <majorminer_types.hpp>




namespace majorminer
{
  class QVariable;
  class QModel;
  class QPolynomial;
  class QConstraint;
  typedef std::pair<QVariable*, QVariable*> QVariablePair;

  class QVariable
  {
    public:
      QVariable(fuint32_t idx) : m_idx(idx){}
      fuint32_t getIdx() const { return m_idx; }

    private:
      fuint32_t m_idx;
  };

  typedef double qcoeff_t;
  typedef PairHashFunc<QVariable*, QVariable*> QVariablePairHashFunc;
  typedef std::vector<QVariable*> QVariableVec;
  typedef std::vector<QConstraint*> QConstraintVec;
  typedef std::unordered_map<QVariablePair, qcoeff_t, QVariablePairHashFunc> QVariablePairMap;


  class QPolynomial
  {
    public:
      QPolynomial(){}
      void addTerm(qcoeff_t coeff) { updateTerm(nullptr, nullptr, coeff); }
      void addTerm(QVariable& var, qcoeff_t coeff) { updateTerm(&var, nullptr, coeff); }
      void addTerm(QVariable& varI, QVariable& varJ, qcoeff_t coeff) { updateTerm(&varI, &varJ, coeff); }
      void addTermMap(const QVariablePairMap& map, qcoeff_t multiplier = 1.0);
      void addTerms(QVariableVec& vars, qcoeff_t coeff);
      size_t nbTerms() const { return m_terms.size(); }
      QVariablePairMap square() const;
      graph_t getConnectivityGraph() const;
      const QVariablePairMap& getTermMap() const { return m_terms; }

      friend QPolynomial operator+=(QPolynomial& lhs, const QPolynomial& rhs)
      {
        lhs.addTermMap(rhs.m_terms);
        return lhs;
      }

    private:
      void updateTerm(QVariablePair& p, qcoeff_t coeff) { m_terms[p] += coeff; }
      void updateTerm(QVariable* varI, QVariable* varJ, qcoeff_t coeff);
      QVariablePair orderedPair(QVariable* varI, QVariable* varJ) const;

    private:
      QVariablePairMap m_terms;
  };

  enum QConstraintType
  {
    EQUAL,
    LOWER_EQUAL,
    GREATER_EQUAL
  };

  class QConstraint
  {
    public:
      QConstraint(QConstraintType type, qcoeff_t rhs, qcoeff_t penalty)
        : m_type(type), m_rhs(rhs), m_penalty(penalty){}
      void addTerm(qcoeff_t coeff) { m_rhs += coeff; }
      void addTerm(QVariable& var, qcoeff_t coeff) { m_polynomial.addTerm(var, coeff); }
      void addTerms(QVariableVec& vars, qcoeff_t coeff) { m_polynomial.addTerms(vars, coeff); }
      qcoeff_t getPenalty() const { return m_penalty; }
      const QPolynomial& getPolynomial() const { return m_polynomial; }
      qcoeff_t getRhs() const { return m_rhs; }
      QConstraintType getType() const { return m_type; }

      friend QPolynomial& operator+=(QPolynomial& poly, QConstraint& cons)
      {
        QPolynomial temp{};
        temp += cons.getPolynomial();
        temp.addTerm(-cons.getRhs());

        auto map = temp.square();
        poly.addTermMap(map, cons.getPenalty());
        return poly;
      }

    private:
      QPolynomial m_polynomial;
      QConstraintType m_type;
      qcoeff_t m_rhs;
      qcoeff_t m_penalty;
  };

  enum AlternativeConstraintType
  {
    EQUAL_ONE,
    LEQ_ONE,
    GEQ_ONE,
    ABSORPTION
  };

  class QModel
  {
    public:
      QModel(){}
      ~QModel();
      QVariableVec createBinaryVars(fuint32_t n);
      QVariable* createBinaryVar();
      QConstraint* createConstraint(QConstraintType type, qcoeff_t rhs, qcoeff_t penalty);
      QPolynomial& getObjective(){ return m_objective; }
      graph_t operator()();
      QPolynomial reformulate();

    private:
      void reformulateConstraint(QConstraint& constraint, QPolynomial& poly);
      void reformulateAlternative(QConstraint& constraint, QPolynomial& poly, AlternativeConstraintType type, fuint32_t n);
      QVariable* normalRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty);
      QVariable* negatedRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty);
      QVariable* partialNegatedRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty);
      QVariable* smallAbsorptionConstraint(QVariable* x1, QVariable* x2, qcoeff_t penalty, QVariable* y = nullptr);

      template<bool x1Negated = false, bool x2Negated = false>
      QVariable* addRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty)
      { // normal rosenberg polynomial y = x1 * x2 <=> P * ( 3y + x1*x2 - 2yx1 - 2yx2 )
        auto* y = createBinaryVar();
        qcoeff_t yCoeff = 3 + (x1Negated ? -2 : 0) + (x2Negated ? -2 : 0);
        poly.addTerm(*y, yCoeff * penalty);
        if (x1Negated) poly.addTerm(*x1, *y, 2 * penalty);
        else poly.addTerm(*x1, *y, -2.0 * penalty);

        if (x2Negated) poly.addTerm(*x2, *y, 2 * penalty);
        else poly.addTerm(*x2, *y, -2.0 * penalty);

        poly.addTerm(*x1, *x2, penalty * (x1Negated == x2Negated ? 1 : -1));

        if (x1Negated && x2Negated)
        { // both negated
          poly.addTerm(penalty);
          poly.addTerm(*x1, -penalty);
          poly.addTerm(*x2, -penalty);
        }
        else if (x1Negated) poly.addTerm(*x2, penalty);
        else if (x2Negated) poly.addTerm(*x1, penalty);
        return y;
      }

    private:
      QPolynomial m_objective;
      QVariableVec m_variables;
      QConstraintVec m_constraints;
      QConstraintVec m_internalConstraints;
  };

}



#endif