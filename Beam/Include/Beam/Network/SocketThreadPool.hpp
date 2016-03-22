#ifndef BEAM_SOCKETTHREADPOOL_HPP
#define BEAM_SOCKETTHREADPOOL_HPP
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Network/Network.hpp"

namespace Beam {
namespace Network {

  /*! \class SocketThreadPool
      \brief Provides the thread pool used by a group of socket Channels.
   */
  class SocketThreadPool : private boost::noncopyable {
    public:

      //! Constructs a SocketThreadPool.
      SocketThreadPool();

      //! Constructs a SocketThreadPool.
      /*!
        \param threadCount The number of threads to use.
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
      std::vector<boost::thread> m_threads;

      boost::asio::io_service& GetService();
  };

  inline SocketThreadPool::SocketThreadPool()
      : SocketThreadPool(boost::thread::hardware_concurrency()) {}

  inline SocketThreadPool::SocketThreadPool(std::size_t threadCount)
      : m_work(m_service) {
    for(std::size_t i = 0; i < threadCount; ++i) {
      m_threads.emplace_back(
        [=] {
          m_service.run();
        });
    }
  }

  inline SocketThreadPool::~SocketThreadPool() {
    m_service.stop();
    for(auto& thread : m_threads) {
      thread.join();
    }
  }

  inline boost::asio::io_service& SocketThreadPool::GetService() {
    return m_service;
  }
}
}

#endif
