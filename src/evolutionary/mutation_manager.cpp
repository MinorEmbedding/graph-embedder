#include "muation_manager.hpp"
#include "majorminer.hpp"

using namespace majorminer;


void MutationManager::mutate()
{
  prepare();

  // one thread integrating mutations, other threads preparing new mutations
  auto& prepQueue = m_prepQueue;
  auto& incorporationQueue = m_incorporationQueue;
  std::thread prep{ [&](){
    MutationPtr mutation;
    while(!prepQueue.empty() || !incorporationQueue.empty())
    {
      bool success = prepQueue.try_pop(mutation);
      if (!success) continue;
      bool valid = mutation->prepare();
      if (!valid) continue;
      else
      {
        incorporationQueue.push(std::move(mutation));
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
    }
  }
}
