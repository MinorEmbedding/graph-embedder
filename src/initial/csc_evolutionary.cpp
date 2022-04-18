#include "initial/csc_evolutionary.hpp"

#include <common/utils.hpp>
#include <common/cut_vertex.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/time_measurement.hpp>

#define POPULATION_SIZE 5
#define ITERATION_LIMIT 10
#define MAX_NEW_VERTICES 15
#define REDUCE_ITERATION_COEFFICIENT 1

using namespace majorminer;

namespace
{
  template<typename T>
  void swapPointers(T*& from, T*& to)
  {
    T*& temp = to;
    to = from;
    from = temp;
  }
}

EvolutionaryCSCReducer::EvolutionaryCSCReducer(EmbeddingState& state,
  vertex_t sourceVertex)
  : m_state(state), m_sourceVertex(sourceVertex), m_wasPlaced(true),
    m_improved(false), m_visualizer(nullptr), m_threadManager(state.getThreadManager())
{
  initialize();
}

EvolutionaryCSCReducer::EvolutionaryCSCReducer(EmbeddingState& state,
  const nodeset_t& initial, vertex_t sourceVertex)
    : m_state(state), m_sourceVertex(sourceVertex), m_wasPlaced(false),
      m_improved(false), m_visualizer(nullptr), m_threadManager(state.getThreadManager())
{
  initialize(initial);
}

void EvolutionaryCSCReducer::optimize()
{
  if (!m_expansionPossible) return;
  Vector<CSCIndividual>* current = &m_populationA;
  Vector<CSCIndividual>* next = &m_populationB;

  for (fuint32_t iteration = 0; iteration < ITERATION_LIMIT; ++iteration)
  {
    CHRONO_STUFF(t1,t2,diff1,OPTIMIZE, optimizeIteration(*current);)
    if (m_visualizer != nullptr) visualize(iteration + 1, current);

    if (iteration + 1 != ITERATION_LIMIT)
    {
      CHRONO_STUFF(t3,t4,diff2,GENERATE_POP,
      bool success = createNextGeneration(*current, *next);
      if (!success) break;)
      swapPointers(current, next);
    }
  }
  if (m_visualizer != nullptr) visualize(FUINT32_UNDEF, nullptr);
  PRINT_TIME(TIME_MUTATION)
  PRINT_TIME(TIME_REDUCE)
  PRINT_TIME(OPTIMIZE)
  PRINT_TIME(GENERATE_POP)
  std::cout << "Threads " << m_threadManager.getAvailableThreads() << std::endl;
}

#define CREATE_STRING(vertices) \
  ss << "Final solution with fitness " << getFitness(vertices) << " and size " << vertices.size() << ".";

#define CREATE_IT_STRING(vertices) \
  ss << "Iteration " << iteration << " individual " << (idx+1) << " has fitness "  << getFitness(vertices) << " and size " << vertices.size() << ".";


void EvolutionaryCSCReducer::visualize(fuint32_t iteration, Vector<CSCIndividual>* population)
{
  if (!isDefined(iteration))
  {
    std::stringstream ss;
    CREATE_STRING(m_bestSuperVertex);
    embedding_mapping_t adjusted = replaceMapping(m_state.getMapping(), m_bestSuperVertex, m_sourceVertex);
    m_visualizer->draw(adjusted, ss.str().c_str());
  }
  else
  {
    std::stringstream ss;
    for (fuint32_t idx = 0; idx < population->size(); ++idx)
    {
      const auto& placement = population->at(idx).getSuperVertex();
      CREATE_IT_STRING(placement)
      embedding_mapping_t adjusted = replaceMapping(m_state.getMapping(), placement, m_sourceVertex);
      m_visualizer->draw(adjusted, ss.str().c_str());
      ss.str(std::string());
    }
  }

}


void EvolutionaryCSCReducer::initialize()
{
  const auto& mapping = m_state.getMapping();
  const auto& reverse = m_state.getReverseMapping();
  auto range = mapping.equal_range(m_sourceVertex);
  for (auto mappedIt = range.first; mappedIt != range.second; ++mappedIt)
  {
    m_bestSuperVertex.insert(mappedIt->second);
    fuint32_t nbMapped = reverse.count(mappedIt->second);
    m_vertexFitness.insert(std::make_pair(mappedIt->second,
      nbMapped >= 1 ? nbMapped - 1 : 0));
  }

  setup();
}

void EvolutionaryCSCReducer::initialize(const nodeset_t& initial)
{
  m_bestSuperVertex.insert(initial.begin(), initial.end());

  const auto& reverse = m_state.getReverseMapping();
  for (vertex_t target : m_bestSuperVertex)
  {
    m_vertexFitness.insert(std::make_pair(target,
      reverse.count(target)));
  }
  setup();
}

void EvolutionaryCSCReducer::setup()
{
  if (!canExpand()) return;

  const auto& mapping = m_state.getMapping();

  // Prepare adjacent source vertices
  m_state.iterateSourceGraphAdjacent(m_sourceVertex, [&](vertex_t adjacentSource){
    if (mapping.contains(adjacentSource)) m_adjacentSourceVertices.insert(adjacentSource);
  });

  // Prepare intial "m_adjacentSources" adjacency list
  for (vertex_t target : m_bestSuperVertex) prepareVertex(target, false);

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
  for (auto& individual : m_populationB)
  {
    individual.initialize(*this, m_sourceVertex);
  }
}

bool EvolutionaryCSCReducer::canExpand()
{
  // Check whether there is some unoccupied adjacent target vertex
  const auto& targetRemaining = m_state.getRemainingTargetNodes();
  m_expansionPossible = false;
  for (vertex_t mappedTarget : m_bestSuperVertex)
  {
    m_state.iterateTargetGraphAdjacentBreak(mappedTarget,
      [&](fuint32_t adjTarget){
        if (!m_bestSuperVertex.contains(adjTarget) && targetRemaining.contains(adjTarget))
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
  #define MULTITHREADED 1
  #if MULTITHREADED == 1
  /*for (auto& parent : parentPopulation)
  {
    m_threadManager.run( [&]() { parent.optimize(); });
  }
  m_threadManager.wait();*/
  #else
  for (auto& parent : parentPopulation) parent.optimize();
  #endif
  // sort parent population
  std::sort(parentPopulation.begin(), parentPopulation.end(), std::less<CSCIndividual>());

  size_t newBestFitness = parentPopulation[0].getFitness();
  size_t size = parentPopulation.at(0).getSolutionSize();
  if (newBestFitness < m_bestFitness ||
    (newBestFitness == m_bestFitness && size < m_bestSuperVertex.size()))
  { // adopt new superior solution
    m_bestFitness = newBestFitness;
    m_bestSuperVertex.clear();
    const auto& newSuperVertex = parentPopulation[0].getSuperVertex();
    m_bestSuperVertex.insert(newSuperVertex.begin(), newSuperVertex.end());
    m_improved = true;
  }
}

bool EvolutionaryCSCReducer::createNextGeneration(Vector<CSCIndividual>& parentPopulation,
  Vector<CSCIndividual>& childPopulation)
{
  fuint32_t remainingAttemps = 5 * POPULATION_SIZE;
  for (fuint32_t idx = 3; idx < POPULATION_SIZE && remainingAttemps > 0; --remainingAttemps)
  {
    const CSCIndividual* parentA = tournamentSelection(parentPopulation);
    const CSCIndividual* parentB = tournamentSelection(parentPopulation);
    bool success = childPopulation[idx].fromCrossover(*parentA, *parentB);
    if (success) idx++;
    else std::cout << "Dead crossover!" << std::endl;
  }
  return remainingAttemps > 0;
}

const CSCIndividual* EvolutionaryCSCReducer::tournamentSelection(const Vector<CSCIndividual>& parentPopulation)
{
  fuint32_t max = POPULATION_SIZE - 1;

  const CSCIndividual* individualA = &parentPopulation.at(m_random.getRandomUint(max));
  const CSCIndividual* individualB = &parentPopulation.at(m_random.getRandomUint(max));
  return *individualA < *individualB ? individualA : individualB;
}


void EvolutionaryCSCReducer::prepareVertex(vertex_t target, bool count)
{
  m_prepareLock.lock();
  if (!m_preparedVertices.contains(target))
  {
    m_temporary.clear();
    m_state.iterateTargetAdjacentReverseMapping(target,
      [&](vertex_t adjacentSource){
        if (m_adjacentSourceVertices.contains(adjacentSource)) m_temporary.insert(adjacentSource);
    });
    m_state.iterateReverseMapping(target, [&](vertex_t source){
        if (m_adjacentSourceVertices.contains(source)) m_temporary.insert(source);
    });
    for (vertex_t source : m_temporary)
    {
      m_adjacentSources.insert(std::make_pair(target, source));
    }
    const auto& reverse = m_state.getReverseMapping();
    if (count) m_vertexFitness.insert(std::make_pair(target, reverse.count(target)));
    m_preparedVertices.insert(target);
  }
  m_prepareLock.unlock();
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

size_t EvolutionaryCSCReducer::getFitness(const nodeset_t& placement) const
{
  size_t fitness = 0;
  VertexNumberMap::const_iterator findIt;
  for (auto vertex : placement)
  {
    findIt = m_vertexFitness.find(vertex);
    if (findIt != m_vertexFitness.end()) fitness += findIt->second;
  }
  return fitness;
}

size_t EvolutionaryCSCReducer::getFitness(vertex_t target) const
{
  auto findIt = m_vertexFitness.find(target);
  return findIt == m_vertexFitness.end() ? 0 : findIt->second;
}

bool EvolutionaryCSCReducer::isRemoveable(VertexNumberMap& connectivity, vertex_t target) const
{
  auto range = m_adjacentSources.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    if (connectivity[it->second] <= 1) return false;
  }
  return true;
}

void EvolutionaryCSCReducer::removeVertex(VertexNumberMap& connectivity, vertex_t target) const
{
  auto range = m_adjacentSources.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    auto findIt = connectivity.find(it->second);
    if (findIt != connectivity.end()) findIt->second--;
  }
}

bool CSCIndividual::isConnected() const
{
  for (const auto& connect : m_connectivity)
  {
    if (connect.second == 0) return false;
  }
  return true;
}

void CSCIndividual::initialize(EvolutionaryCSCReducer& reducer, vertex_t sourceVertex)
{
  m_reducer = &reducer;
  m_state = &reducer.m_state;
  m_sourceVertex = sourceVertex;
  m_random = std::make_unique<RandomGen>();
}



void CSCIndividual::fromInitial(const nodeset_t& placement)
{
  m_done = false;
  m_superVertex.clear();
  m_superVertex.insert(placement.begin(), placement.end());

  setupConnectivity();
}

bool CSCIndividual::fromCrossover(const CSCIndividual& individualA, const CSCIndividual& individualB)
{
  m_done = false;
  m_superVertex.clear();
  const auto& superVertexA = individualA.getSuperVertex();
  const auto& superVertexB = individualB.getSuperVertex();
  if (!overlappingSets(superVertexA, superVertexB)
    && !areSetsConnected(*m_state, superVertexA, superVertexB))
  {
    m_done = true;
    return false;
  }
  m_superVertex.insert(superVertexA.begin(), superVertexA.end());
  m_superVertex.insert(superVertexB.begin(), superVertexB.end());
  setupConnectivity();
  return true;
}

void CSCIndividual::printConnectivity() const
{
  printVertexNumberMap(m_connectivity);
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
  if (m_done) return;
  CHRONO_STUFF(t1,t2,diff1,TIME_MUTATION, mutate();)

  CHRONO_STUFF(t3,t4,diff2, TIME_REDUCE, reduce();)
  m_fitness = m_reducer->getFitness(m_superVertex);
  m_done = true;
}


size_t CSCIndividual::getSolutionSize() const
{
  return m_superVertex.size();
}

size_t CSCIndividual::getFitness() const
{
  return m_fitness;
}

void CSCIndividual::mutate()
{
  m_temporarySet.clear();
  for (vertex_t vertex : m_superVertex)
  {
    m_state->iterateFreeTargetAdjacent(vertex,
      [&](vertex_t adjacentTarget){
        if(!m_superVertex.contains(adjacentTarget))
        {
          m_temporarySet.insert(adjacentTarget);
        }
    });
  }
  if (m_temporarySet.empty()) return;

  vertex_t startVertex = m_random->getRandomVertex(m_temporarySet);

  if (!isDefined(startVertex)) return;
  m_temporarySet.clear();
  clearStack(m_iteratorStack);

  const auto& targetGraph = m_state->getTargetAdjGraph();
  const auto& remaining = m_state->getRemainingTargetNodes();

  fuint32_t numberAdded = 1;
  addVertex(startVertex);
  m_iteratorStack.push(targetGraph.equal_range(startVertex));
  while(!m_iteratorStack.empty() && numberAdded <= MAX_NEW_VERTICES)
  {
    auto& top = m_iteratorStack.top();
    if (top.first == top.second)
    {
      m_iteratorStack.pop();
      continue;
    }
    vertex_t adjacent = top.first->second;
    if (remaining.contains(adjacent) && !m_superVertex.contains(adjacent))
    { // add node
      top.first++;
      addVertex(adjacent);
      numberAdded++;
      m_iteratorStack.push(targetGraph.equal_range(adjacent));
    }
    else
    {
      top.first++;
    }
  }
}

void CSCIndividual::reduce()
{
  if (m_superVertex.size() <= 1) return;
  m_vertexVector.resize(m_superVertex.size());
  fuint32_t idx = 0;
  for (vertex_t vertex : m_superVertex) m_vertexVector[idx++] = vertex;
  fuint32_t vectorSize = m_vertexVector.size();

  // Try greedily reducing overlap vertices
  for (idx = 0; idx < vectorSize;)
  {
    vertex_t* current = &m_vertexVector[idx];
    if (m_reducer->getFitness(*current) != 0 && tryRemove(*current))
    {
      *current = m_vertexVector.back();
      m_vertexVector.resize(--vectorSize);
    }
    else idx++;
  }

  // Try reducing all other vertices
  fuint32_t maxIterations = REDUCE_ITERATION_COEFFICIENT * m_vertexVector.size();
  vectorSize = m_vertexVector.size();
  for (fuint32_t iteration = 0; iteration < maxIterations; ++iteration)
  {
    fuint32_t randomIdx = m_random->getRandomUint(vectorSize - 1);
    vertex_t* current = &m_vertexVector[randomIdx];
    if (!m_superVertex.contains(*current) || tryDfsRemove(*current, iteration))
    {
      *current = m_vertexVector.back();
      m_vertexVector.resize(--vectorSize);
    }
  }

  for (idx = 0; idx < vectorSize; ++idx)
  {
    if (m_superVertex.contains(m_vertexVector[idx])) tryRemove(m_vertexVector[idx]);
  }
}

void CSCIndividual::addVertex(vertex_t target)
{
  if (m_superVertex.contains(target)) return;
  m_reducer->addConnectivity(m_connectivity, target);
  m_superVertex.insert(target);
}

bool CSCIndividual::tryRemove(vertex_t target)
{
  // Remove if not a cut vertex
  if (!m_reducer->isRemoveable(m_connectivity, target)) return false;

  m_temporarySet.clear();
  m_temporarySet.insert(m_superVertex.begin(), m_superVertex.end());
  if (!isCutVertex(*m_state, m_temporarySet, target))
  {
    m_reducer->removeVertex(m_connectivity, target);
    m_superVertex.unsafe_erase(target);
    return true;
  }
  m_temporarySet.clear();
  return false;
}

bool CSCIndividual::tryDfsRemove(vertex_t target, fuint32_t& iteration)
{
  if (!tryRemove(target)) return false;

  const auto& targetGraph = m_state->getTargetAdjGraph();
  clearStack(m_iteratorStack);
  m_iteratorStack.push(targetGraph.equal_range(target));
  while(!m_iteratorStack.empty())
  {
    auto& top = m_iteratorStack.top();
    if (top.first == top.second) m_iteratorStack.pop();
    else if (m_superVertex.contains(top.first->second))
    {
      vertex_t adjacent = top.first->second;
      top.first++;
      iteration++;
      bool success = tryRemove(adjacent);
      if (success) m_iteratorStack.push(targetGraph.equal_range(adjacent));
    }
    else top.first++;
  }
  return true;
}

