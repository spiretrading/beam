#ifndef BEAM_TIMERTHREADPOOL_HPP
#define BEAM_TIMERTHREADPOOL_HPP
#include <vector>
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
      std::vector<boost::thread> m_threads;

      boost::asio::io_service& GetService();
  };

  inline TimerThreadPool::TimerThreadPool()
      : m_work(m_service) {
    for(std::size_t i = 0; i < boost::thread::hardware_concurrency(); ++i) {
      boost::thread serviceThread(std::bind(
        static_cast<std::size_t (boost::asio::io_service::*)()>(
        &boost::asio::io_service::run), std::ref(m_service)));
      m_threads.push_back(std::move(serviceThread));
    }
  }

  inline TimerThreadPool::TimerThreadPool(std::size_t threadCount)
      : m_work(m_service) {
    for(std::size_t i = 0; i < threadCount; ++i) {
      boost::thread serviceThread(std::bind(
        static_cast<std::size_t (boost::asio::io_service::*)()>(
        &boost::asio::io_service::run), std::ref(m_service)));
      m_threads.push_back(std::move(serviceThread));
    }
  }

  inline TimerThreadPool::~TimerThreadPool() {
    m_service.stop();
    for(boost::thread& thread : m_threads) {
      thread.join();
    }
  }

  inline boost::asio::io_service& TimerThreadPool::GetService() {
    return m_service;
  }
}
}

#endif
