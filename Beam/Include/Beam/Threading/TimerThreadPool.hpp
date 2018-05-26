#ifndef BEAM_TIMER_THREAD_POOL_HPP
#define BEAM_TIMER_THREAD_POOL_HPP
#include <memory>
#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class TimerThreadPool
      \brief Provides the thread pool used by Timer implementations.
   */
  class TimerThreadPool : private boost::noncopyable {
    public:

      //! Constructs a TimerThreadPool.
      TimerThreadPool();

      //! Constructs a TimerThreadPool.
      /*!
        \param threadCount The number of threads to use.
      */
      TimerThreadPool(std::size_t threadCount);

      ~TimerThreadPool();

    private:
      friend class LiveTimer;
      boost::asio::io_service m_service;
      boost::asio::io_service::work m_work;
      std::size_t m_threadCount;
      std::unique_ptr<boost::thread[]> m_threads;

      boost::asio::io_service& GetService();
  };

  inline TimerThreadPool::TimerThreadPool()
      : TimerThreadPool(boost::thread::hardware_concurrency()) {}

  inline TimerThreadPool::TimerThreadPool(std::size_t threadCount)
      : m_work(m_service),
        m_threadCount(threadCount),
        m_threads(std::make_unique<boost::thread[]>(m_threadCount)) {
    for(std::size_t i = 0; i < m_threadCount; ++i) {
      m_threads[i] = boost::thread(
        [=] {
          m_service.run();
        });
    }
  }

  inline TimerThreadPool::~TimerThreadPool() {
    m_service.stop();
    for(std::size_t i = 0; i < m_threadCount; ++i) {
      m_threads[i].join();
    }
  }

  inline boost::asio::io_service& TimerThreadPool::GetService() {
    return m_service;
  }
}
}

#endif
