#include "evolutionary/mutation_manager.hpp"

#include <evolutionary/mutation_extend.hpp>
#include <evolutionary/mutation_frontier_shifting.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_manager.hpp>


using namespace majorminer;


void MutationManager::operator()()
{
  prepare();
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
      bool valid = mutation->prepare();
      if (!valid)
      {
        remaining--;
        continue;
      }
      else
      {
        free.lock();
        runningPreps++;
        free.unlock();
        incorporationQueue.push(std::move(mutation));
        runningPreps--;
      }
    }
  }};
  incorporate();
  prep.join();
  m_embeddingManager.synchronize();
  // std::cout << "Ending mutations with " << m_prepQueue.unsafe_size() << " in prep queue and " << m_incorporationQueue.size() << " in incorporation queue." << std::endl;
}

void MutationManager::prepare()
{
  m_embeddingManager.clear();
  m_prepQueue.clear();
  m_incorporationQueue.clear();

  // insert potential mutations
  auto lastNode = m_embeddingManager.getLastNode();
  if (lastNode != (fuint32_t)-1)
  {
    prepareMutations(lastNode);
  }
  m_numberRemaining = m_prepQueue.unsafe_size();
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
  {
    bool success = m_incorporationQueue.try_pop(mutation);
    if (!success) continue;

    if (!mutation->isValid())
    {
      std::cout << "Invalid mutation. Requeuing." << std::endl;
      m_prepQueue.push(std::move(mutation));
      // m_numberRemaining--; // REMOVE AGAIN!
    }
    else
    {
      mutation->execute();
      m_numberRemaining--;
      m_wait = true;
    }
    if (m_wait)
    {
      m_free.lock();
      while(m_runningPreps != 0) continue;
      m_embeddingManager.synchronize();
      m_wait = false;
      m_free.unlock();
    }
  }
  m_done = true;
}
