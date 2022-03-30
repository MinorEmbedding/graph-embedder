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


vertex_t RandomGen::getRandomVertex(const nodeset_t& vertices)
{
  if (vertices.empty()) return FUINT32_UNDEF;
  fuint32_t idx = getRandomUint(vertices.size() - 1);
  for (auto vertex : vertices)
  {
    if (idx-- == 0) return vertex;
  }
  return FUINT32_UNDEF;
}