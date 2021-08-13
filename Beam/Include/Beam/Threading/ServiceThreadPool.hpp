#ifndef BEAM_SERVICE_THREAD_POOL_HPP
#define BEAM_SERVICE_THREAD_POOL_HPP
#include <memory>
#include <boost/asio/io_service.hpp>
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
      boost::asio::io_service m_service;
      boost::asio::io_service::work m_work;
      std::size_t m_threadCount;
      std::unique_ptr<boost::thread[]> m_threads;

      ServiceThreadPool();
      ServiceThreadPool(const ServiceThreadPool&) = delete;
      ServiceThreadPool& operator =(const ServiceThreadPool&) = delete;
      boost::asio::io_service& GetService();
  };

  inline ServiceThreadPool::~ServiceThreadPool() {
    m_service.stop();
    for(auto i = std::size_t(0); i < m_threadCount; ++i) {
      m_threads[i].join();
    }
  }

  inline ServiceThreadPool::ServiceThreadPool()
      : m_work(m_service),
        m_threadCount(boost::thread::hardware_concurrency()),
        m_threads(std::make_unique<boost::thread[]>(m_threadCount)) {
    for(auto i = std::size_t(0); i < m_threadCount; ++i) {
      m_threads[i] = boost::thread([this] {
        m_service.run();
      });
    }
  }

  inline boost::asio::io_service& ServiceThreadPool::GetService() {
    return m_service;
  }
}

#endif
