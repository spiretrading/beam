#ifndef BEAM_TO_PYTHON_TIME_CLIENT_HPP
#define BEAM_TO_PYTHON_TIME_CLIENT_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/TimeService/TimeService.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"

namespace Beam {
namespace TimeService {

  /*! \class ToPythonTimeClient
      \brief Wraps a TimeClient for use with Python.
      \tparam ClientType The type of TimeClient to wrap.
   */
  template<typename ClientType>
  class ToPythonTimeClient : public VirtualTimeClient {
    public:

      //! The type of TimeClient to wrap.
      using Client = ClientType;

      //! Constructs a ToPythonTimeClient.
      /*!
        \param client The TimeClient to wrap.
      */
      ToPythonTimeClient(std::unique_ptr<Client> client);

      virtual ~ToPythonTimeClient() override final;

      //! Returns the TimeClient being wrapped.
      const Client& GetClient() const;

      //! Returns the TimeClient being wrapped.
      Client& GetClient();

      virtual boost::posix_time::ptime GetTime() override final;

      virtual void Open() override final;

      virtual void Close() override final;

    private:
      std::unique_ptr<Client> m_client;
      IO::OpenState m_openState;

      void Shutdown();
  };

  //! Makes a ToPythonTimeClient.
  /*!
    \param client The TimeClient to wrap.
  */
  template<typename Client>
  auto MakeToPythonTimeClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonTimeClient<Client>>(std::move(client));
  }

  template<typename ClientType>
  ToPythonTimeClient<ClientType>::ToPythonTimeClient(
      std::unique_ptr<Client> client)
      : m_client{std::move(client)} {}

  template<typename ClientType>
  ToPythonTimeClient<ClientType>::~ToPythonTimeClient() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    Close();
    m_client.reset();
  }

  template<typename ClientType>
  const typename ToPythonTimeClient<ClientType>::Client&
      ToPythonTimeClient<ClientType>::GetClient() const {
    return *m_client;
  }

  template<typename ClientType>
  typename ToPythonTimeClient<ClientType>::Client&
      ToPythonTimeClient<ClientType>::GetClient() {
    return *m_client;
  }

  template<typename ClientType>
  boost::posix_time::ptime ToPythonTimeClient<ClientType>::GetTime() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    return m_client->GetTime();
  }

  template<typename ClientType>
  void ToPythonTimeClient<ClientType>::Open() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_client->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ClientType>
  void ToPythonTimeClient<ClientType>::Close() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ClientType>
  void ToPythonTimeClient<ClientType>::Shutdown() {
    m_client->Close();
    m_openState.SetClosed();
  }
}
}

#endif
