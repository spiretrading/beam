#ifndef BEAM_TO_PYTHON_TIME_CLIENT_HPP
#define BEAM_TO_PYTHON_TIME_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/TimeService/TimeClient.hpp"

namespace Beam::Python {

  /**
   * Wraps a TimeClient for use within Python.
   * @tparam C The type of TimeClient to wrap.
   */
  template<IsTimeClient C>
  class ToPythonTimeClient {
    public:

      /** The type of TimeClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonTimeClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonTimeClient(Args&&... args);

      ~ToPythonTimeClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      boost::posix_time::ptime get_time();
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonTimeClient(const ToPythonTimeClient&) = delete;
      ToPythonTimeClient& operator =(const ToPythonTimeClient&) = delete;
  };

  template<typename TimeClient>
  ToPythonTimeClient(TimeClient&&) ->
    ToPythonTimeClient<std::remove_cvref_t<TimeClient>>;

  template<IsTimeClient C>
  template<typename... Args>
  ToPythonTimeClient<C>::ToPythonTimeClient(Args&&... args)
    : m_client((pybind11::gil_scoped_release(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsTimeClient C>
  ToPythonTimeClient<C>::~ToPythonTimeClient() {
    auto release = pybind11::gil_scoped_release();
    m_client.reset();
  }

  template<IsTimeClient C>
  typename ToPythonTimeClient<C>::Client& ToPythonTimeClient<C>::get() {
    return *m_client;
  }

  template<IsTimeClient C>
  const typename ToPythonTimeClient<C>::Client&
      ToPythonTimeClient<C>::get() const {
    return *m_client;
  }

  template<IsTimeClient C>
  boost::posix_time::ptime ToPythonTimeClient<C>::get_time() {
    auto release = pybind11::gil_scoped_release();
    return m_client->get_time();
  }

  template<IsTimeClient C>
  void ToPythonTimeClient<C>::close() {
    auto release = pybind11::gil_scoped_release();
    m_client->close();
  }
}

#endif
