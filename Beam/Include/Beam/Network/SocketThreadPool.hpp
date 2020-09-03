#ifndef BEAM_SOCKET_THREAD_POOL_HPP
#define BEAM_SOCKET_THREAD_POOL_HPP
#include <memory>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Network/Network.hpp"

namespace Beam::Network {

  /** Provides the thread pool used by a group of socket Channels. */
  class SocketThreadPool {
    public:

      /** Constructs a SocketThreadPool. */
      SocketThreadPool();

      /**
       * Constructs a SocketThreadPool.
       * @param threadCount The number of threads to use.
       */
      SocketThreadPool(std::size_t threadCount);

      ~SocketThreadPool();

    private:
      friend class MulticastSocket;
      friend class SecureSocketChannel;
      friend class TcpServerSocket;
      friend class TcpSocketChannel;
      friend class UdpSocket;
      boost::asio::io_service m_service;
      boost::asio::io_service::work m_work;
      std::size_t m_threadCount;
      std::unique_ptr<boost::thread[]> m_threads;

      SocketThreadPool(const SocketThreadPool&) = delete;
      SocketThreadPool& operator =(const SocketThreadPool&) = delete;
      boost::asio::io_service& GetService();
  };

  inline SocketThreadPool::SocketThreadPool()
    : SocketThreadPool(boost::thread::hardware_concurrency()) {}

  inline SocketThreadPool::SocketThreadPool(std::size_t threadCount)
      : m_work(m_service),
        m_threadCount(threadCount),
        m_threads(std::make_unique<boost::thread[]>(m_threadCount)) {
    for(std::size_t i = 0; i < m_threadCount; ++i) {
      m_threads[i] = boost::thread([=] {
        m_service.run();
      });
    }
  }

  inline SocketThreadPool::~SocketThreadPool() {
    m_service.stop();
    for(std::size_t i = 0; i < m_threadCount; ++i) {
      m_threads[i].join();
    }
  }

  inline boost::asio::io_service& SocketThreadPool::GetService() {
    return m_service;
  }
}

#endif
