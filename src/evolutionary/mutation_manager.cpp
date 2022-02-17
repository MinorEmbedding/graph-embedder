#include "muation_manager.hpp"
#include "majorminer.hpp"

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
  const auto& wait = m_wait;
  auto& free = m_free;
  std::thread prep{ [&](){
    MutationPtr mutation;
    while(!done)
    {
    //std::cout << "mutate " << prepQueue.empty() << ", " << incorporationQueue.empty() << std::endl;
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
  auto lastNode = m_suite.m_embeddingManager.getLastNode();
  if (lastNode != (fuint32_t)-1)
  {
    prepareAffectedExtendCandidates(lastNode);
  }

}
void MutationManager::prepareAffectedExtendCandidates(fuint32_t node)
{
  nodeset_t extendAffected{};
  auto mapped = m_suite.m_mapping.equal_range(node);
  for (auto mapIt = mapped.first; mapIt != mapped.second; ++mapIt)
  {
    auto adjacentRange = m_suite.m_target.equal_range(mapIt->second);
    for (auto adjIt = adjacentRange.first; adjIt != adjacentRange.second; ++adjIt)
    {
      auto revMapped = m_suite.m_reverseMapping.equal_range(adjIt->second);
      for (auto revIt = revMapped.first; revIt != revMapped.second; ++revIt)
      {
        extendAffected.insert(revIt->second);
      }
    }
  }
  for (auto extendCandidate : extendAffected)
  {
    m_prepQueue.push(std::make_unique<MutationExtend>(&m_suite, extendCandidate));
  }
}

void MutationManager::incorporate()
{
  while(!m_prepQueue.empty() || !m_incorporationQueue.empty())
  {
    //std::cout << "Incorporate " << m_prepQueue.empty() << ", " << m_incorporationQueue.empty() << std::endl;
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
      std::cout << "Mutation done" << std::endl;
      m_wait = true;
    }
    if (m_wait)
    {
      m_free.lock();
      while(m_runningPreps != 0) continue;
      std::cout << "Synchronizing" << std::endl;
      m_suite.m_embeddingManager.synchronize();
      std::cout << "Synchronization done." << std::endl;
      m_wait = false;
      m_free.unlock();
    }
  }
  m_done = true;
}
