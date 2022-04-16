#ifndef __MAJORMINER_THREAD_MANAGER_HPP_
#define __MAJORMINER_THREAD_MANAGER_HPP_

#include <majorminer_types.hpp>
#include <thread-pool/thread_pool.hpp>

namespace majorminer
{

  class ThreadManager
  {
    public:
      ThreadManager(){}

      fuint32_t getAvailableThreads() const { return m_pool.get_thread_count(); }

      template<typename Functor>
      void runMultiple(const Functor& func, fuint32_t n)
      {
        for (fuint32_t i = 0; i < n; ++i) m_pool.push_task(func);
      }

      template<typename Functor>
      void run(const Functor& func)
      {
        m_pool.push_task(func);
      }

      void wait() { m_pool.wait_for_tasks(); }

    private:
      thread_pool m_pool;

  };

}


#endif