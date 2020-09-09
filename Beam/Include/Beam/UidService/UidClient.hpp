#ifndef BEAM_UID_CLIENT_HPP
#define BEAM_UID_CLIENT_HPP
#include <boost/thread/mutex.hpp>
#include <cstdint>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/UidService/UidService.hpp"
#include "Beam/UidService/UidServices.hpp"

namespace Beam::UidService {

  /**
   * Client used to generate unique identifiers.
   * @param <B> The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class UidClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = GetTryDereferenceType<B>;

      /**
       * Constructs a UidClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit UidClient(BF&& clientBuilder);

      ~UidClient();

      /** Returns the next available UID. */
      std::uint64_t LoadNextUid();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      std::uint64_t m_nextUid;
      std::uint64_t m_lastUid;
      std::uint64_t m_blockSize;
      mutable boost::mutex m_mutex;
      mutable Threading::ConditionVariable m_uidsAvailableCondition;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      IO::OpenState m_openState;
  };

  template<typename B>
  template<typename BF>
  UidClient<B>::UidClient(BF&& clientBuilder)
      : m_nextUid(1),
        m_lastUid(0),
        m_blockSize(10),
        m_clientHandler(std::forward<BF>(clientBuilder)) {
    RegisterUidServices(Store(m_clientHandler.GetSlots()));
  }

  template<typename B>
  UidClient<B>::~UidClient() {
    Close();
  }

  template<typename B>
  std::uint64_t UidClient<B>::LoadNextUid() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_nextUid > m_lastUid) {
      if(m_nextUid == m_lastUid + 1) {
        ++m_nextUid;
        auto reserveResult = [&] {
          try {
            auto release = Threading::Release(lock);
            auto client = m_clientHandler.GetClient();
            return client->template SendRequest<ReserveUidsService>(
              m_blockSize);
          } catch(const std::exception&) {
            --m_nextUid;
            m_uidsAvailableCondition.notify_one();
            throw;
          }
        }();
        m_nextUid = reserveResult;
        m_lastUid = m_nextUid + m_blockSize - 1;
        m_blockSize = 2 * m_blockSize;
        m_uidsAvailableCondition.notify_all();
      } else {
        m_uidsAvailableCondition.wait(lock);
      }
    }
    auto uid = m_nextUid;
    ++m_nextUid;
    return uid;
  }

  template<typename B>
  void UidClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_openState.Close();
  }
}

#endif
