#ifndef BEAM_TO_PYTHON_TIME_CLIENT_HPP
#define BEAM_TO_PYTHON_TIME_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/TimeService/TimeClient.hpp"

namespace Beam::TimeService {

  /**
   * Wraps a TimeClient for use with Python.
   * @param <C> The type of TimeClient to wrap.
   */
  template<typename C>
  class ToPythonTimeClient {
    public:

      /** The type of TimeClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonTimeClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonTimeClient(Args&&... args);

      ToPythonTimeClient(ToPythonTimeClient&&) = default;

      ~ToPythonTimeClient();

      /** Returns the wrapped TimeClient. */
      const Client& GetClient() const;

      /** Returns the wrapped TimeClient. */
      Client& GetClient();

      boost::posix_time::ptime GetTime();

      void Close();

      ToPythonTimeClient& operator =(ToPythonTimeClient&&) = default;

    private:
      boost::optional<Client> m_client;

      ToPythonTimeClient(const ToPythonTimeClient&) = delete;
      ToPythonTimeClient& operator =(const ToPythonTimeClient&) = delete;
  };

  template<typename TimeClient>
  ToPythonTimeClient(TimeClient&&) ->
    ToPythonTimeClient<std::decay_t<TimeClient>>;

  template<typename C>
  template<typename... Args>
  ToPythonTimeClient<C>::ToPythonTimeClient(Args&&... args)
    : m_client((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonTimeClient<C>::~ToPythonTimeClient() {
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
