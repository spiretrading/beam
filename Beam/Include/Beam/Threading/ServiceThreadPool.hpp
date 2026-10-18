#ifndef BEAM_SERVICE_THREAD_POOL_HPP
#define BEAM_SERVICE_THREAD_POOL_HPP
#include <algorithm>
#include <memory>
#include <thread>
#include <boost/asio/io_context.hpp>
#include "Beam/Network/UdpSocketReceiverConcept.hpp"
#include "Beam/Utilities/DllExport.hpp"
#include "Beam/Utilities/Singleton.hpp"

namespace Beam {
  template<IsUdpSocketReceiver R>
  class BasicMulticastSocket;
  class SecureSocketChannel;
  class TcpServerSocket;
  class TcpSocketChannel;
  template<IsUdpSocketReceiver R>
  class BasicUdpSocket;

  /** Wraps a list of ASIO worker threads. */
  class BEAM_EXPORT_DLL ServiceThreadPool :
      public Singleton<ServiceThreadPool> {
    public:
      ~ServiceThreadPool();

    private:
      template<IsUdpSocketReceiver R>
      friend class BasicMulticastSocket;
      friend class Beam::SecureSocketChannel;
      friend class Beam::TcpServerSocket;
      friend class Beam::TcpSocketChannel;
      template<IsUdpSocketReceiver R>
      friend class BasicUdpSocket;
      friend class LiveTimer;
      friend class Singleton<ServiceThreadPool>;
      boost::asio::io_context m_service;
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        m_work;
      std::size_t m_thread_count;
      std::unique_ptr<std::thread[]> m_threads;

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
        m_thread_count(std::max(1u, std::thread::hardware_concurrency())),
        m_threads(std::make_unique<std::thread[]>(m_thread_count)) {
    try {
      for(auto i = std::size_t(0); i < m_thread_count; ++i) {
        m_threads[i] = std::thread([this] {
          m_service.run();
        });
      }
    } catch(...) {
      m_service.stop();
      for(auto i = std::size_t(0); i < m_thread_count; ++i) {
        if(m_threads[i].joinable()) {
          m_threads[i].join();
        }
      }
      throw;
    }
  }

  inline boost::asio::io_context& ServiceThreadPool::get_context() {
    return m_service;
  }
}

#endif
