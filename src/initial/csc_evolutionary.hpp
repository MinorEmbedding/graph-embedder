#ifndef __MAJORMINER_CSC_EVOLUTIONARY_HPP_
#define __MAJORMINER_CSC_EVOLUTIONARY_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  class CSCIndividual
  {
    friend EvolutionaryCSCReducer;
    public:
      CSCIndividual(): m_reducer(nullptr) {}

      void initialize(EvolutionaryCSCReducer& reducer, vertex_t sourceVertex);

      void fromInitial(const nodeset_t& placement);
      void fromCrossover(const CSCIndividual& individualA, const CSCIndividual& individualB);
      void optimize();
      const nodeset_t& getSuperVertex() const { return m_superVertex; }

    private:
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
      vertex_t m_sourceVertex;
      nodeset_t m_superVertex;
      VertexNumberMap m_connectivity;
      fuint32_t m_fitness;
  };

  class EvolutionaryCSCReducer
  {
    friend CSCIndividual;
    public:
      EvolutionaryCSCReducer(const EmbeddingState& state, vertex_t sourceVertex);

      bool optimize();

    private:
      void initialize();
      bool canExpand();
      void initializePopulations();
      void optimizeIteration(Vector<CSCIndividual>& parentPopulation);
      void createNextGeneration(Vector<CSCIndividual>& parentPopulation, Vector<CSCIndividual>& childPopulation);
      void prepareVertex(vertex_t target);

    private:
      void addConnectivity(VertexNumberMap& connectivity, vertex_t target);
      fuint32_t getFitness(const nodeset_t& placement) const;
      const nodeset_t& getAdjacentSourceVertices() const { return m_adjacentSourceVertices; }

    private:
      const EmbeddingState& m_state;
      vertex_t m_sourceVertex;
      bool m_expansionPossible;

      Vector<CSCIndividual> m_populationA;
      Vector<CSCIndividual> m_populationB;
      adjacency_list_t m_adjacentSources;
      nodeset_t m_adjacentSourceVertices;
      VertexNumberMap m_vertexFitness;
      nodeset_t m_preparedVertices;

      nodeset_t m_bestSuperVertex;
      size_t m_bestFitness;

      nodeset_t m_temporary;
  };

}



#endif