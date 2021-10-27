#ifndef __MAJORMINER_QUBO_MODELLING_HPP_
#define __MAJORMINER_QUBO_MODELLING_HPP_

#include <vector>
#include <unordered_map>
#include <atomic>

#include <majorminer_types.hpp>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>




namespace majorminer
{
  class QVariable;
  class QModel;
  class QPolynomial;
  class QConstraint;
  class QEnumerationVerifier;
  struct QRosenbergPoly;
  typedef std::pair<QVariable*, QVariable*> QVariablePair;

  class QVariable
  {
    public:
      QVariable(fuint32_t idx) : m_idx(idx){}
      fuint32_t getIdx() const { return m_idx; }

      friend std::ostream& operator<<(std::ostream& os, const QVariable& var)
      { os << "x" << var.m_idx; return os; }

    private:
      fuint32_t m_idx;
  };

  typedef double qcoeff_t;
  typedef PairHashFunc<QVariable*, QVariable*> QVariablePairHashFunc;
  typedef Vector<QVariable*> QVariableVec;
  typedef Vector<QConstraint*> QConstraintVec;
  typedef Vector<QRosenbergPoly> QRosenbergPolyVec;
  typedef UnorderedMap<QVariablePair, qcoeff_t, QVariablePairHashFunc> QVariablePairMap;


  class QPolynomial
  {
    public:
      QPolynomial(){}
      void addTerm(qcoeff_t coeff) { updateTerm(nullptr, nullptr, coeff); }
      void addTerm(QVariable& var, qcoeff_t coeff = 1.0) { updateTerm(&var, nullptr, coeff); }
      void addTerm(QVariable& varI, QVariable& varJ, qcoeff_t coeff = 1.0) { updateTerm(&varI, &varJ, coeff); }
      void addTermMap(const QVariablePairMap& map, qcoeff_t multiplier = 1.0);
      void addTerms(QVariableVec& vars, qcoeff_t coeff = 1.0);
      size_t nbTerms() const { return m_terms.size(); }
      QVariablePairMap square() const;
      graph_t getConnectivityGraph() const;
      const QVariablePairMap& getTermMap() const { return m_terms; }

      friend QPolynomial operator+=(QPolynomial& lhs, const QPolynomial& rhs)
      {
        lhs.addTermMap(rhs.m_terms);
        return lhs;
      }

      friend std::ostream& operator<<(std::ostream& os, const QPolynomial& poly)
      {
        const auto& terms = poly.getTermMap();

        fuint32_t col = 0;
        for (const auto& term : terms)
        {
          if (term.second == 0) continue;
          if (col == 4)
          {
            col = 0;
            os << std::endl;
          }

          if (term.second > 0) os << " + ";
          else os << " - ";

          os << std::abs(term.second);
          const auto& vars = term.first;
          if (vars.first != nullptr) os << " * " << *vars.first;
          if (vars.second != nullptr) os << " * " << *vars.second;
          col++;
        }
        return os;
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
      void addTerm(QVariable& var, qcoeff_t coeff = 1.0) { m_polynomial.addTerm(var, coeff); }
      void addTerms(QVariableVec& vars, qcoeff_t coeff = 1.0) { m_polynomial.addTerms(vars, coeff); }
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

  struct QRosenbergPoly
  {
    QRosenbergPoly(QVariable* x1, QVariable* x2, QVariable* y, bool x1Neg, bool x2Neg, qcoeff_t penalty)
      : m_x1(x1), m_x2(x2), m_y(y), m_x1Negated(x1Neg), m_x2Negated(x2Neg), m_penalty(penalty) {}

    QVariable* m_x1;
    QVariable* m_x2;
    QVariable* m_y;
    bool m_x1Negated;
    bool m_x2Negated;
    qcoeff_t m_penalty;
  };

  class QModel
  {
    friend QEnumerationVerifier;
    public:
      QModel(){}
      ~QModel();
      QVariableVec createBinaryVars(fuint32_t n);
      QVariable* createBinaryVar();
      QConstraint* createConstraint(QConstraintType type, qcoeff_t rhs, qcoeff_t penalty);
      QPolynomial& getObjective(){ return m_objective; }
      graph_t operator()();
      QPolynomial reformulate();

      template<bool x1Negated = false, bool x2Negated = false>
      QVariable* addRosenbergPolynomial(QVariable* x1, QVariable* x2, qcoeff_t penalty)
      {
        auto* y = createBinaryVar();
        m_rosenbergVec.push_back(QRosenbergPoly{x1,x2,y,x1Negated,x2Negated, penalty});
        return y;
      }

    private:
      void reformulateRosenberg(QPolynomial& poly, const QRosenbergPoly& rosenberg) const;
      void reformulateConstraint(QConstraint& constraint, QPolynomial& poly);
      void reformulateAlternative(QConstraint& constraint, QPolynomial& poly, AlternativeConstraintType type, fuint32_t n);
      QVariable* normalRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty);
      QVariable* negatedRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty);
      QVariable* partialNegatedRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty);
      QVariable* smallAbsorptionConstraint(QVariable* x1, QVariable* x2, qcoeff_t penalty, QVariable* y = nullptr);
      void insertInternalLEQ1Constraint(QVariable* x1, QVariable* x2, qcoeff_t penalty);
      template<bool x1Negated = false, bool x2Negated = false>
      void addRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, QVariable* y, qcoeff_t penalty) const
      { // normal rosenberg polynomial y = x1 * x2 <=> P * ( 3y + x1*x2 - 2yx1 - 2yx2 )
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
      }

      template<bool x1Negated = false, bool x2Negated = false>
      QVariable* addRosenbergPolynomial(QPolynomial& poly, QVariable* x1, QVariable* x2, qcoeff_t penalty)
      {
        auto* y = addRosenbergPolynomial<x1Negated, x2Negated>(x1, x2, penalty);
        addRosenbergPolynomial<x1Negated, x2Negated>(poly, x1, x2, y, penalty);
        return y;
      }

    private:
      QPolynomial m_objective;
      QVariableVec m_variables;
      QConstraintVec m_constraints;
      QConstraintVec m_internalConstraints;
      QRosenbergPolyVec m_rosenbergVec;
  };

  struct QTerm
  {
    QTerm(QVariable* x, QVariable* y, qcoeff_t coeff)
      : m_x(x), m_y(y), m_coeff(coeff) {}

    bool quadratic() const { return m_x != nullptr && m_y != nullptr; }
    bool linear() const { return m_x != nullptr && m_y == nullptr; }

    QVariable* m_x;
    QVariable* m_y;
    qcoeff_t m_coeff;
  };
  typedef Vector<QTerm> QTermVec;

  // Exponential time complexity, use only for small problems!
  class QEnumerationVerifier
  {
    public:
      QEnumerationVerifier(QModel& model, const QPolynomial& reformulated, qcoeff_t minErrorVal, bool firstStop, bool dumpOnError = false)
        : m_model(&model), m_minErrorVal(minErrorVal), m_firstStop(firstStop), m_dumpOnError(dumpOnError)
        { copyReformulated(reformulated); }

      bool verify();

    private:
      bool testSetting() const;
      bool testConstraint(const QConstraint& cons) const;
      bool testRosenberg(const QRosenbergPoly& rosenberg) const;
      qcoeff_t evaluateObjective() const;
      void copyReformulated(const QPolynomial& reformulated);
      void dumpSetting() const;

    private:
      const QModel* m_model;
      QTermVec m_reformulated;
      qcoeff_t m_minErrorVal; // what should be the min error if constraint is violated
      bool m_firstStop; // stop on first error?
      Vector<bool> m_setting;
      bool m_dumpOnError;
  };

}



#endif