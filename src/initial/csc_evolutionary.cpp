#include "initial/csc_evolutionary.hpp"

#include <common/utils.hpp>
#include <common/embedding_state.hpp>

#define POPULATION_SIZE 10
#define ITERATION_LIMIT 10

using namespace majorminer;

EvolutionaryCSCReducer::EvolutionaryCSCReducer(const EmbeddingState& state, fuint32_t sourceVertex)
  : m_state(state), m_sourceVertex(sourceVertex)
{
  initialize();
}

bool EvolutionaryCSCReducer::optimize()
{
  Vector<CSCIndividual>* current = &m_populationA;
  Vector<CSCIndividual>* next = &m_populationB;

  for (fuint32_t iteration = 0; iteration < ITERATION_LIMIT; ++iteration)
  {
    optimizeIteration(*current);

    if (iteration + 1 != ITERATION_LIMIT)
    {
      createNextGeneration(*current, *next);
      std::swap(current, next);
    }
  }

  return false;
}

void EvolutionaryCSCReducer::initialize()
{
  if (!canExpand()) return;

  const auto& mapping = m_state.getMapping();
  const auto& reverse = m_state.getReverseMapping();
  auto range = mapping.equal_range(m_sourceVertex);
  for (auto mappedIt = range.first; mappedIt != range.second; ++mappedIt)
  {
    m_bestSuperVertex.insert(mappedIt->second);
    m_vertexFitness.insert(std::make_pair(mappedIt->second,
      reverse.count(mappedIt->second) - 1));
  }
  m_preparedVertices.insert(m_bestSuperVertex.begin(), m_bestSuperVertex.end());

  // Prepare adjacent source vertices
  m_state.iterateSourceGraphAdjacent(m_sourceVertex, [&](vertex_t adjacentSource){
    if (mapping.contains(adjacentSource)) m_adjacentSourceVertices.insert(adjacentSource);
  });

  // Prepare intial "adjacentSource" adjacency list
  nodepairset_t targetAdjSourcePairs;
  for (auto target : m_bestSuperVertex)
  {
    m_state.iterateTargetAdjacentReverseMapping(target, [&](vertex_t adjSource){
      if (m_adjacentSourceVertices.contains(adjSource))
      {
        targetAdjSourcePairs.insert(std::make_pair(target, adjSource));
      }
    });
  }

  for (auto& adj : targetAdjSourcePairs) m_adjacentSources.insert(adj);

  m_bestFitness = getFitness(m_bestSuperVertex);
  initializePopulations();
}

void EvolutionaryCSCReducer::initializePopulations()
{
  m_populationA.resize(POPULATION_SIZE);
  m_populationB.resize(POPULATION_SIZE);

  for (auto& individual : m_populationA)
  {
    individual.initialize(*this, m_sourceVertex);
    individual.fromInitial(m_bestSuperVertex); // at this point m_bestSuperVertex is initial placement
  }
  for (auto& individual : m_populationA)
  {
    individual.initialize(*this, m_sourceVertex);
  }
}

bool EvolutionaryCSCReducer::canExpand()
{
  // Check whether there is some unoccupied adjacent target vertex
  auto range = m_state.getMapping().equal_range(m_sourceVertex);
  const auto& targetRemaining = m_state.getRemainingTargetNodes();
  m_expansionPossible = false;
  for (auto it = range.first; it != range.second; ++it)
  {
    m_state.iterateTargetGraphAdjacentBreak(it->second,
      [&](fuint32_t adjTarget){
        if (targetRemaining.contains(adjTarget))
        {
          m_expansionPossible = true;
        }
        return m_expansionPossible;
    });
    if (m_expansionPossible) return true;
  }
  return false;
}


void EvolutionaryCSCReducer::optimizeIteration(Vector<CSCIndividual>& parentPopulation)
{
  // optimize all in parent population
  for (auto& parent : parentPopulation) parent.optimize();

  // sort parent population
  std::sort(parentPopulation.begin(), parentPopulation.end(), std::less<CSCIndividual>());

  size_t newBestFitness = parentPopulation[0].getFitness();
  size_t size = parentPopulation[0].getSolutionSize();
  if (newBestFitness < m_bestFitness ||
    (newBestFitness == m_bestFitness && size < m_bestSuperVertex.size()))
  { // adopt new superior solution
    m_bestFitness = newBestFitness;
    m_bestSuperVertex.clear();
    const auto& newSuperVertex = parentPopulation[0].getSuperVertex();
    m_bestSuperVertex.insert(newSuperVertex.begin(), newSuperVertex.end());
  }
}

void EvolutionaryCSCReducer::createNextGeneration(Vector<CSCIndividual>& /* parentPopulation */,
  Vector<CSCIndividual>& /* childPopulation */)
{

}

void EvolutionaryCSCReducer::prepareVertex(vertex_t target)
{
  m_temporary.clear();
  m_state.iterateTargetAdjacentReverseMapping(target, [&](fuint32_t adjacentSource){
    if (m_adjacentSourceVertices.contains(adjacentSource)) m_temporary.insert(adjacentSource);
  });
  for (auto source : m_temporary)
  {
    m_adjacentSources.insert(std::make_pair(target, source));
  }
}

void EvolutionaryCSCReducer::addConnectivity(VertexNumberMap& connectivity, vertex_t target)
{
  if (!m_preparedVertices.contains(target)) prepareVertex(target);

  auto range = m_adjacentSources.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    connectivity[it->second]++;
  }
}

void CSCIndividual::initialize(EvolutionaryCSCReducer& reducer, vertex_t sourceVertex)
{
  m_reducer = &reducer;
  m_sourceVertex = sourceVertex;
}

void CSCIndividual::fromInitial(const nodeset_t& placement)
{
  m_superVertex.clear();
  m_superVertex.insert(placement.begin(), placement.end());

  setupConnectivity();
}

void CSCIndividual::fromCrossover(const CSCIndividual& individualA, const CSCIndividual& individualB)
{
  m_superVertex.clear();
  const auto& superVertexA = individualA.getSuperVertex();
  const auto& superVertexB = individualB.getSuperVertex();
  if (!overlappingSets(superVertexA, superVertexB))
  {
    // TODO: check whether connected
  }
  m_superVertex.insert(superVertexA.begin(), superVertexA.end());
  m_superVertex.insert(superVertexB.begin(), superVertexB.end());

  setupConnectivity();
}

void CSCIndividual::setupConnectivity()
{
  const auto& adjacentSourceVertices = m_reducer->getAdjacentSourceVertices();
  for (auto adjSource : adjacentSourceVertices)
  {
    m_connectivity[adjSource] = 0;
  }

  for (auto target : m_superVertex)
  {
    m_reducer->addConnectivity(m_connectivity, target);
  }
}

void CSCIndividual::optimize()
{

}


size_t CSCIndividual::getSolutionSize() const
{
  return m_superVertex.size();
}

size_t CSCIndividual::getFitness() const
{
  return m_fitness;
}
