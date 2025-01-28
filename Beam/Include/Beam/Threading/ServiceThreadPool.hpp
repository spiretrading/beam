#ifndef BEAM_SERVICE_THREAD_POOL_HPP
#define BEAM_SERVICE_THREAD_POOL_HPP
#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Network/Network.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam::Threading {

  /** Wraps a list of ASIO worker threads. */
  class ServiceThreadPool : public Singleton<ServiceThreadPool> {
    public:
      ~ServiceThreadPool();

    private:
      friend class Beam::Network::MulticastSocket;
      friend class Beam::Network::SecureSocketChannel;
      friend class Beam::Network::TcpServerSocket;
      friend class Beam::Network::TcpSocketChannel;
      friend class Beam::Network::UdpSocket;
      friend class LiveTimer;
      friend class Singleton<ServiceThreadPool>;
      boost::asio::io_context m_service;
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        m_work;
      std::size_t m_threadCount;
      std::unique_ptr<boost::thread[]> m_threads;

      ServiceThreadPool();
      ServiceThreadPool(const ServiceThreadPool&) = delete;
      ServiceThreadPool& operator =(const ServiceThreadPool&) = delete;
      boost::asio::io_context& GetContext();
  };

  inline ServiceThreadPool::~ServiceThreadPool() {
    m_service.stop();
    for(auto i = std::size_t(0); i < m_threadCount; ++i) {
      m_threads[i].join();
    }
  }

  inline ServiceThreadPool::ServiceThreadPool()
      : m_work(boost::asio::make_work_guard(m_service)),
        m_threadCount(boost::thread::hardware_concurrency()),
        m_threads(std::make_unique<boost::thread[]>(m_threadCount)) {
    for(auto i = std::size_t(0); i < m_threadCount; ++i) {
      m_threads[i] = boost::thread([this] {
        m_service.run();
      });
    }
  }

  inline boost::asio::io_context& ServiceThreadPool::GetContext() {
    return m_service;
  }
}

#endif
