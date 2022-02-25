#ifndef __MAJORMINER_RANDOM_GEN_HPP_
#define __MAJORMINER_RANDOM_GEN_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  class RandomGen
  {
    public:
      RandomGen(){}
      fuint32_t getRandomUint(fuint32_t upper);

      template<typename T>
      void shuffle(T* data, fuint32_t size)
      {
        if (size == 0) return;
        while(!m_shuffleLock.try_lock()) {}

        std::shuffle(data, data + size, m_shuffleGenerator);

        m_shuffleLock.unlock();
      }

    private:
      std::mutex m_lock;
      std::mutex m_shuffleLock;
      std::default_random_engine m_generator;
      std::default_random_engine m_shuffleGenerator;
  };



}


#endif