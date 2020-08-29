#ifndef BEAM_TO_PYTHON_TIME_CLIENT_HPP
#define BEAM_TO_PYTHON_TIME_CLIENT_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"

namespace Beam::TimeService {

  /**
   * Wraps a TimeClient for use with Python.
   * @param <C> The type of TimeClient to wrap.
   */
  template<typename C>
  class ToPythonTimeClient final : public VirtualTimeClient {
    public:

      /** The type of TimeClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonTimeClient.
       * @param client The TimeClient to wrap.
       */
      ToPythonTimeClient(std::unique_ptr<Client> client);

      ~ToPythonTimeClient() override;

      /** Returns the TimeClient being wrapped. */
      const Client& GetClient() const;

      /** Returns the TimeClient being wrapped. */
      Client& GetClient();

      boost::posix_time::ptime GetTime() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonTimeClient.
   * @param client The TimeClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonTimeClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonTimeClient<Client>>(std::move(client));
  }

  template<typename C>
  ToPythonTimeClient<C>::ToPythonTimeClient(std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonTimeClient<C>::~ToPythonTimeClient() {
    Close();
    auto release = Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonTimeClient<C>::Client&
      ToPythonTimeClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonTimeClient<C>::Client& ToPythonTimeClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  boost::posix_time::ptime ToPythonTimeClient<C>::GetTime() {
    auto release = Python::GilRelease();
    return m_client->GetTime();
  }

  template<typename C>
  void ToPythonTimeClient<C>::Close() {
    auto release = Python::GilRelease();
    m_client->Close();
  }
}

#endif
