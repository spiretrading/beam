#ifndef BEAM_SERVICE_UID_CLIENT_HPP
#define BEAM_SERVICE_UID_CLIENT_HPP
#include <boost/thread/mutex.hpp>
#include <cstdint>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidServices.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Implements a UidClient using Beam services.
   * @tparam B The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class ServiceUidClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = dereference_t<B>;

      /**
       * Constructs a UidClient.
       * @param client_builder Initializes the ServiceProtocolClientBuilder.
       */
      template<Initializes<B> BF>
      explicit ServiceUidClient(BF&& client_builder);

      ~ServiceUidClient();

      std::uint64_t load_next_uid();
      void close();

    private:
      mutable boost::mutex m_mutex;
      Beam::ServiceProtocolClientHandler<B> m_client_handler;
      std::uint64_t m_next_uid;
      std::uint64_t m_last_uid;
      std::uint64_t m_block_size;
      mutable ConditionVariable m_uids_available_condition;
      OpenState m_open_state;
  };

  template<typename B>
  template<Initializes<B> BF>
  ServiceUidClient<B>::ServiceUidClient(BF&& client_builder)
      try : m_client_handler(std::forward<BF>(client_builder)),
            m_next_uid(1),
            m_last_uid(0),
            m_block_size(10) {
    register_uid_services(out(m_client_handler.get_slots()));
  } catch(const std::exception&) {
    std::throw_with_nested(
      ConnectException("Failed to connect to the UID server."));
  }

  template<typename B>
  ServiceUidClient<B>::~ServiceUidClient() {
    close();
  }

  template<typename B>
  std::uint64_t ServiceUidClient<B>::load_next_uid() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_next_uid > m_last_uid) {
      if(m_next_uid == m_last_uid + 1) {
        ++m_next_uid;
        auto reserve_result = [&] {
          try {
            auto releaser = release(lock);
            return service_or_throw_with_nested([&] {
              auto client = m_client_handler.get_client();
              return client->template send_request<ReserveUidsService>(
                m_block_size);
            }, "Failed to load UIDs.");
          } catch(const std::exception&) {
            --m_next_uid;
            m_uids_available_condition.notify_one();
            throw;
          }
        }();
        m_next_uid = reserve_result;
        m_last_uid = m_next_uid + m_block_size - 1;
        m_block_size = 2 * m_block_size;
        m_uids_available_condition.notify_all();
      } else {
        m_uids_available_condition.wait(lock);
      }
    }
    auto uid = m_next_uid;
    ++m_next_uid;
    return uid;
  }

  template<typename B>
  void ServiceUidClient<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client_handler.close();
    m_open_state.close();
  }
}

#endif
