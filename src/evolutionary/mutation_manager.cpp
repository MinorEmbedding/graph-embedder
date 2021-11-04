#include "muation_manager.hpp"

using namespace majorminer;


void MutationManager::mutate()
{
  prepare();

  // one thread integrating mutations, other threads preparing new mutations
  auto& prepQueue = m_prepQueue;
  auto& incorporationQueue = m_incorporationQueue;
  std::thread prep{ [&](){

  }};
  incorporate();
  prep.join();
}

void MutationManager::prepare()
{
  m_prepQueue.clear();
  m_incorporationQueue.clear();

  // insert potential mutations
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
      m_prepQueue.push(mutation);
    }
    else
    {
      mutation->execute();
    }
  }
}
