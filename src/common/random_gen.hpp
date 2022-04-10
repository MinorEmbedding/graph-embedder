#ifndef __MAJORMINER_RANDOM_GEN_HPP_
#define __MAJORMINER_RANDOM_GEN_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  class RandomGen
  {
    public:
      RandomGen();
      fuint32_t getRandomUint(fuint32_t upper);

      template<typename T>
      void shuffle(T* data, fuint32_t size)
      {
        if (size == 0) return;
        while(!m_shuffleLock.try_lock()) {}

        std::shuffle(data, data + size, m_shuffleGenerator);

        m_shuffleLock.unlock();
      }

      vertex_t getRandomVertex(const nodeset_t& vertices);

    private:
      std::mutex m_lock;
      std::mutex m_shuffleLock;
      std::random_device m_rdGen;
      std::random_device m_rdShuffle;
      std::default_random_engine m_generator;
      std::default_random_engine m_shuffleGenerator;
  };

  template<typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
  struct ProbabilisticDecision
  {
    public:
      ProbabilisticDecision() : m_gen(m_rd()), m_dist(static_cast<T>(0.0), static_cast<T>(1.0)) {}
      T operator()() { return m_dist(m_gen); }
      bool operator()(T value) { return m_dist(m_gen) < value; }

    private:
      std::random_device m_rd;
      std::mt19937 m_gen;
      std::uniform_real_distribution<T> m_dist;
  };
}


#endif