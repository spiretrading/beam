#ifndef BEAM_UIDCLIENT_HPP
#define BEAM_UIDCLIENT_HPP
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <cstdint>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/UidService/UidService.hpp"
#include "Beam/UidService/UidServices.hpp"

namespace Beam {
namespace UidService {

  /*! \class UidClient
      \brief Client used to generate unique identifiers.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class UidClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder =
        typename TryDereferenceType<ServiceProtocolClientBuilderType>::type;

      //! Constructs a UidClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      UidClient(ClientBuilderForward&& clientBuilder);

      ~UidClient();

      //! Returns the next available UID.
      std::uint64_t LoadNextUid();

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      std::uint64_t m_nextUid;
      std::uint64_t m_lastUid;
      std::uint64_t m_blockSize;
      mutable boost::mutex m_uidMutex;
      mutable Threading::ConditionVariable m_uidsAvailableCondition;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  UidClient<ServiceProtocolClientBuilderType>::UidClient(
      ClientBuilderForward&& clientBuilder)
      : m_nextUid(1),
        m_lastUid(0),
        m_blockSize(10),
        m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    RegisterUidServices(Store(m_clientHandler.GetSlots()));
  }

  template<typename ServiceProtocolClientBuilderType>
  UidClient<ServiceProtocolClientBuilderType>::~UidClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  std::uint64_t UidClient<ServiceProtocolClientBuilderType>::LoadNextUid() {
    boost::unique_lock<boost::mutex> lock(m_uidMutex);
    while(m_nextUid > m_lastUid) {
      if(m_nextUid == m_lastUid + 1) {
        ++m_nextUid;
        std::uint64_t reserveResult;
        {
          try {
            auto release = Threading::Release(lock);
            auto client = m_clientHandler.GetClient();
            reserveResult = client->template SendRequest<ReserveUidsService>(
              m_blockSize);
          } catch(const std::exception&) {
            --m_nextUid;
            m_uidsAvailableCondition.notify_one();
            throw;
          }
        }
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

  template<typename ServiceProtocolClientBuilderType>
  void UidClient<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void UidClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void UidClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }
}
}

#endif
