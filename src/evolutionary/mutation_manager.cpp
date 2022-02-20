#include "evolutionary/mutation_manager.hpp"

#include <evolutionary/mutation_extend.hpp>
#include <evolutionary/mutation_frontier_shifting.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_manager.hpp>


using namespace majorminer;


void MutationManager::mutate()
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
  // const auto& wait = m_wait;
  auto& free = m_free;
  std::thread prep{ [&](){
    MutationPtr mutation;
    while(!done)
    {
      bool success = prepQueue.try_pop(mutation);
      if (!success) continue;
      bool valid = mutation->prepare();
      if (!valid) continue;
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
}

void MutationManager::prepare()
{
  m_prepQueue.clear();
  m_incorporationQueue.clear();

  // insert potential mutations
  auto lastNode = m_embeddingManager.getLastNode();
  if (lastNode != (fuint32_t)-1)
  {
    prepareAffectedExtendCandidates(lastNode);
  }

}
void MutationManager::prepareAffectedExtendCandidates(fuint32_t node)
{
  nodeset_t extendAffected{};
  m_state.iterateSourceMappingAdjacent<false>(node, [&](fuint32_t target){
    m_state.iterateReverseMapping(target, [&](fuint32_t revSourceNode){
      extendAffected.insert(revSourceNode);
    });
  });

  for (auto extendCandidate : extendAffected)
  {
    m_prepQueue.push(std::make_unique<MutationExtend>(m_state, m_embeddingManager, extendCandidate));
  }
}

void MutationManager::incorporate()
{
  while(!m_prepQueue.empty() || !m_incorporationQueue.empty())
  {
    MutationPtr mutation;
    bool success = m_incorporationQueue.try_pop(mutation);
    if (!success) continue;

    if (!mutation->isValid())
    {
      m_prepQueue.push(std::move(mutation));
    }
    else
    {
      mutation->execute();
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
