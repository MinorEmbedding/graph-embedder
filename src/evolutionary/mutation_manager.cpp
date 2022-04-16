#include "evolutionary/mutation_manager.hpp"

#include <evolutionary/mutation_extend.hpp>
#include <evolutionary/mutation_frontier_shifting.hpp>
#include <evolutionary/mutation_reduce_overlap.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_manager.hpp>

#include <common/debug_utils.hpp>


using namespace majorminer;


void MutationManager::operator()(bool finalIteration)
{
  if (!finalIteration) prepare();
  else prepareFinal();
  m_done = false;
  m_runningPreps = 0;
  m_wait = false;

  // one thread integrating mutations, other threads preparing new mutations
  auto& prepQueue = m_prepQueue;
  auto& incorporationQueue = m_incorporationQueue;
  const auto& done = m_done;
  auto& runningPreps = m_runningPreps;
  auto& remaining = m_numberRemaining;
  // const auto& wait = m_wait;
  auto& free = m_free;
  std::thread prep{ [&](){
    MutationPtr mutation;
    while(!done)
    {
      bool success = prepQueue.try_pop(mutation);
      if (!success) continue;
      else
      {
        free.lock_shared();
        runningPreps++;
        free.unlock_shared();

        // TEST_OUTPUT(Preparing mutation)
        bool valid = mutation->prepare();
        // TEST_OUTPUT(Prepared mutation)
        if (valid) incorporationQueue.push(std::move(mutation));
        else remaining--;
        runningPreps--;
      }
    }
  }};
  incorporate();
  prep.join();
  m_embeddingManager.synchronize();
}

void MutationManager::prepare()
{
  clear();

  // insert potential mutations
  auto lastNode = m_embeddingManager.getLastNode();
  if (lastNode != (fuint32_t)-1)
  {
    prepareMutations(lastNode);
  }
  m_numberRemaining = m_prepQueue.unsafe_size();
}

void MutationManager::prepareFinal()
{
  clear();
  std::vector<fuint32_t> vertices{};
  vertices.reserve(m_state.getNumberSourceVertices());
  const auto& source = m_state.getSourceAdjGraph();

  for (const auto& vertexPair : source)
  {
    if (vertices.empty() || vertices.back() != vertexPair.first)
    {
      vertices.push_back(vertexPair.first);
    }
  }


  RandomGen rand{};
  rand.shuffle(vertices.data(), vertices.size());
  for (auto vertex : vertices)
  {
    m_prepQueue.push(std::make_unique<MutationReduceOverlap>(m_state, m_embeddingManager, vertex));
  }
  // std::cout << "Prepare final with " << vertices.size() <<" vertices." << std::endl;
  m_numberRemaining = m_prepQueue.unsafe_size();
}

void MutationManager::clear()
{
  m_embeddingManager.clear();
  m_prepQueue.clear();
  m_incorporationQueue.clear();
}

void MutationManager::prepareMutations(fuint32_t node)
{
  nodeset_t affected{};
  m_state.iterateSourceMappingAdjacent<false>(node, [&](fuint32_t target, fuint32_t /* */){
    m_state.iterateReverseMapping(target, [&](fuint32_t revSourceNode){
      affected.insert(revSourceNode);
    });
    return false;
  });
  affected.insert(node);

  for (auto candidate : affected)
  {
    m_prepQueue.push(std::make_unique<MutationExtend>(m_state, m_embeddingManager, candidate));
    m_prepQueue.push(std::make_unique<MutationFrontierShifting>(m_state, m_embeddingManager, candidate));
  }
}


void MutationManager::incorporate()
{
  MutationPtr mutation;
  while(m_numberRemaining > 0)
  {// TEST_OUTPUT(popping mutation)
    bool success = m_incorporationQueue.try_pop(mutation);
   // TEST_OUTPUT(done popping mutation)
    if (!success) continue;
  // TEST_OUTPUT(Cheking valid)
    bool valid = mutation->isValid();
  // TEST_OUTPUT(Done checking valid)
    if (!valid && mutation->requeue())
    {
      // std::cout << "Invalid mutation. Requeuing." << std::endl;
      m_prepQueue.push(std::move(mutation));
      continue;
    }
    else if (valid)
    {
      // std::cout << "Starting exec" << std::endl;
      mutation->execute();
      // std::cout << "Done execution" << std::endl;
      m_wait = true;
    }
    m_numberRemaining--;
    if (m_wait)
    {
      m_free.lock();
      while(m_runningPreps != 0) continue;
      // std::cout << "Synchronizing" << std::endl;
      m_embeddingManager.synchronize();
      // std::cout << "Done synchro" << std::endl;
      m_wait = false;
      m_free.unlock();
    }
  }
  m_done = true;
}
