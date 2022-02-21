#include "common/random_gen.hpp"

using namespace majorminer;

fuint32_t RandomGen::getRandomUint(fuint32_t upper)
{
  std::uniform_int_distribution<fuint32_t> distribution(0, upper);
  while(!m_lock.try_lock()) {}

  fuint32_t randomVal = distribution(m_generator);

  m_lock.unlock();
  return randomVal;
}

void RandomGen::shuffle(fuint32_t* data, fuint32_t size)
{
  if (size == 0) return;
  while(!m_shuffleLock.try_lock()) {}

  std::shuffle(data, data + size, m_shuffleGenerator);

  m_shuffleLock.unlock();
}
