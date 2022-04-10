#ifndef __MAJORMINER_CSC_EVOLUTIONARY_HPP_
#define __MAJORMINER_CSC_EVOLUTIONARY_HPP_

#include <majorminer_types.hpp>
#include <common/random_gen.hpp>

namespace majorminer
{

  class CSCIndividual
  {
    friend EvolutionaryCSCReducer;
    public:
      CSCIndividual(): m_reducer(nullptr), m_done(false) {}

      void initialize(EvolutionaryCSCReducer& reducer, vertex_t sourceVertex);

      void fromInitial(const nodeset_t& placement);
      bool fromCrossover(const CSCIndividual& individualA, const CSCIndividual& individualB);
      void optimize();
      const nodeset_t& getSuperVertex() const { return m_superVertex; }

    private:
      void addVertex(vertex_t target);
      bool tryRemove(vertex_t target);
      void mutate();
      void reduce();
      void setupConnectivity();
      size_t getSolutionSize() const;
      size_t getFitness() const;

    public:
      friend bool operator<(const CSCIndividual& in1, const CSCIndividual& in2)
      {
        return in1.getFitness() < in2.getFitness()
          || (in1.getFitness() == in2.getFitness() && in1.getSolutionSize() < in2.getSolutionSize());
      }

    private:
      EvolutionaryCSCReducer* m_reducer;
      const EmbeddingState* m_state;
      vertex_t m_sourceVertex;
      nodeset_t m_superVertex;
      VertexNumberMap m_connectivity;
      size_t m_fitness;

      nodeset_t m_temporarySet;
      Stack<adjacency_list_range_iterator_t> m_iteratorStack;
      Vector<vertex_t> m_vertexVector;
      std::unique_ptr<RandomGen> m_random;

      bool m_done;
  };

  class EvolutionaryCSCReducer
  {
    friend CSCIndividual;
    public:
      EvolutionaryCSCReducer(const EmbeddingState& state, vertex_t sourceVertex);
      EvolutionaryCSCReducer(const EmbeddingState& state, const nodeset_t& initial, vertex_t sourceVertex);

      void optimize();
      const nodeset_t& getPlacement() const { return m_bestSuperVertex; }
      bool foundBetter() const { return m_improved; }

    private:
      void initialize();
      void initialize(const nodeset_t& initial);
      void setup();
      bool canExpand();
      void initializePopulations();
      void optimizeIteration(Vector<CSCIndividual>& parentPopulation);
      bool createNextGeneration(Vector<CSCIndividual>& parentPopulation, Vector<CSCIndividual>& childPopulation);
      void prepareVertex(vertex_t target);
      const CSCIndividual* tournamentSelection(const Vector<CSCIndividual>& parentPopulation);

    private: // called mainly by CSCIndividual
      void addConnectivity(VertexNumberMap& connectivity, vertex_t target);
      bool isRemoveable(VertexNumberMap& connectivity, vertex_t target) const;
      void removeVertex(VertexNumberMap& connectivity, vertex_t target) const;
      size_t getFitness(vertex_t target) const;
      size_t getFitness(const nodeset_t& placement) const;
      const nodeset_t& getAdjacentSourceVertices() const { return m_adjacentSourceVertices; }

    private:
      const EmbeddingState& m_state;
      vertex_t m_sourceVertex;
      bool m_wasPlaced;
      bool m_expansionPossible;
      bool m_improved;

      Vector<CSCIndividual> m_populationA;
      Vector<CSCIndividual> m_populationB;
      adjacency_list_t m_adjacentSources;
      nodeset_t m_adjacentSourceVertices;
      VertexNumberMap m_vertexFitness;
      nodeset_t m_preparedVertices;

      nodeset_t m_bestSuperVertex;
      size_t m_bestFitness;

      nodeset_t m_temporary;
      RandomGen m_random;
  };

}



#endif