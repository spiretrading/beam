#ifndef BEAM_TO_PYTHON_UID_CLIENT_HPP
#define BEAM_TO_PYTHON_UID_CLIENT_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/UidService/UidService.hpp"
#include "Beam/UidService/VirtualUidClient.hpp"

namespace Beam {
namespace UidService {

  /*! \class ToPythonUidClient
      \brief Wraps a UidClient for use with Python.
      \tparam ClientType The type of UidClient to wrap.
   */
  template<typename ClientType>
  class ToPythonUidClient : public VirtualUidClient {
    public:

      //! The type of UidClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonUidClient.
      /*!
        \param client The UidClient to wrap.
      */
      ToPythonUidClient(std::unique_ptr<Client> client);

      virtual ~ToPythonUidClient() override final;

      virtual std::uint64_t LoadNextUid() override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
  };

  //! Makes a ToPythonUidClient.
  /*!
    \param client The UidClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonUidClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonUidClient<Client>>(std::move(client));
  }

  template<typename ClientType>
  ToPythonUidClient<ClientType>::ToPythonUidClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonUidClient<ClientType>::~ToPythonUidClient() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client.reset();
  }

  template<typename ClientType>
  std::uint64_t ToPythonUidClient<ClientType>::LoadNextUid() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->LoadNextUid();
  }

  template<typename ClientType>
  void ToPythonUidClient<ClientType>::Open() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->Open();
  }

  template<typename ClientType>
  void ToPythonUidClient<ClientType>::Close() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_client->Close();
  }
}
}

#endif
