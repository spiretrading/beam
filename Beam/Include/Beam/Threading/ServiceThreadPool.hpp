#ifndef BEAM_SERVICE_THREAD_POOL_HPP
#define BEAM_SERVICE_THREAD_POOL_HPP
#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/thread/thread.hpp>
#include "Beam/Utilities/Singleton.hpp"

namespace Beam {
  class MulticastSocket;
  class SecureSocketChannel;
  class TcpServerSocket;
  class TcpSocketChannel;
  class UdpSocket;

  /** Wraps a list of ASIO worker threads. */
  class ServiceThreadPool : public Singleton<ServiceThreadPool> {
    public:
      ~ServiceThreadPool();

    private:
      friend class Beam::MulticastSocket;
      friend class Beam::SecureSocketChannel;
      friend class Beam::TcpServerSocket;
      friend class Beam::TcpSocketChannel;
      friend class Beam::UdpSocket;
      friend class LiveTimer;
      friend class Singleton<ServiceThreadPool>;
      boost::asio::io_context m_service;
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        m_work;
      std::size_t m_thread_count;
      std::unique_ptr<boost::thread[]> m_threads;

      ServiceThreadPool();
      ServiceThreadPool(const ServiceThreadPool&) = delete;
      ServiceThreadPool& operator =(const ServiceThreadPool&) = delete;
      boost::asio::io_context& get_context();
  };

  inline ServiceThreadPool::~ServiceThreadPool() {
    m_service.stop();
    for(auto i = std::size_t(0); i < m_thread_count; ++i) {
      m_threads[i].join();
    }
  }

  inline ServiceThreadPool::ServiceThreadPool()
      : m_work(boost::asio::make_work_guard(m_service)),
        m_thread_count(boost::thread::hardware_concurrency()),
        m_threads(std::make_unique<boost::thread[]>(m_thread_count)) {
    for(auto i = std::size_t(0); i < m_thread_count; ++i) {
      m_threads[i] = boost::thread([this] {
        m_service.run();
      });
    }
  }

  inline boost::asio::io_context& ServiceThreadPool::get_context() {
    return m_service;
  }
}

#endif
