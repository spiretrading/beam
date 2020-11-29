#ifndef BEAM_TO_PYTHON_CONNECTION_HPP
#define BEAM_TO_PYTHON_CONNECTION_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::IO {

  /**
   * Wraps a Connection for use with Python.
   * @param <C> The type of Connection to wrap.
   */
  template<typename C>
  class ToPythonConnection {
    public:

      /** The type of Connection to wrap. */
      using Connection = C;

      /**
       * Constructs a ToPythonConnection in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonConnection(Args&&... args);

      ToPythonConnection(ToPythonConnection&&) = default;

      ~ToPythonConnection();

      void Close();

      ToPythonConnection& operator =(ToPythonConnection&&) = default;

    private:
      boost::optional<Connection> m_connection;

      ToPythonConnection(const ToPythonConnection&) = delete;
      ToPythonConnection& operator =(const ToPythonConnection&) = delete;
  };

  template<typename Connection>
  ToPythonConnection(Connection&&) ->
    ToPythonConnection<std::decay_t<Connection>>;

  template<typename C>
  template<typename... Args>
  ToPythonConnection<C>::ToPythonConnection(Args&&... args)
    : m_connection((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonConnection<C>::~ToPythonConnection() {
    auto release = Python::GilRelease();
    m_connection.reset();
  }

  template<typename C>
  void ToPythonConnection<C>::Close() {
    auto release = Python::GilRelease();
    m_connection->Close();
  }
}

#endif
