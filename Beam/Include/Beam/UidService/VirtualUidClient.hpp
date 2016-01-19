#ifndef BEAM_VIRTUALUIDCLIENT_HPP
#define BEAM_VIRTUALUIDCLIENT_HPP
#include <cstdint>
#include <memory>
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam {
namespace UidService {

  /*! \class VirtualUidClient
      \brief Provides a pure virtual interface to a UidClient.
   */
  class VirtualUidClient : private boost::noncopyable {
    public:
      virtual ~VirtualUidClient();

      virtual std::uint64_t LoadNextUid() = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualUidClient.
      VirtualUidClient();
  };

  /*! \class WrapperUidClient
      \brief Wraps a UidClient providing it with a virtual interface.
      \tparam ClientType The type of UidClient to wrap.
   */
  template<typename ClientType>
  class WrapperUidClient : public VirtualUidClient {
    public:

      //! The UidClient to wrap.
      typedef typename TryDereferenceType<ClientType>::type Client;

      //! Constructs a WrapperUidClient.
      /*!
        \param client The UidClient to wrap.
      */
      template<typename UidClientForward>
      WrapperUidClient(UidClientForward&& client);

      virtual ~WrapperUidClient();

      virtual std::uint64_t LoadNextUid();

      virtual void Open();

      virtual void Close();

    private:
      typename OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a UidClient into a VirtualUidClient.
  /*!
    \param client The client to wrap.
  */
  template<typename UidClient>
  std::unique_ptr<VirtualUidClient> MakeVirtualUidClient(UidClient&& client) {
    return std::make_unique<WrapperUidClient<UidClient>>(
      std::forward<UidClient>(client));
  }

  inline VirtualUidClient::~VirtualUidClient() {}

  inline VirtualUidClient::VirtualUidClient() {}

  template<typename ClientType>
  template<typename UidClientForward>
  WrapperUidClient<ClientType>::WrapperUidClient(UidClientForward&& client)
      : m_client(std::forward<UidClientForward>(client)) {}

  template<typename ClientType>
  WrapperUidClient<ClientType>::~WrapperUidClient() {}

  template<typename ClientType>
  std::uint64_t WrapperUidClient<ClientType>::LoadNextUid() {
    return m_client->LoadNextUid();
  }

  template<typename ClientType>
  void WrapperUidClient<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperUidClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
