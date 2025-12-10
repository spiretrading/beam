#ifndef BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#define BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/ServerConnection.hpp"

namespace Beam::Python {

  /**
   * Wraps a ServerConnection for use with Python.
   * @tparam C The type of ServerConnection to wrap.
   */
  template<IsServerConnection C>
  class ToPythonServerConnection {
    public:

      /** The type of ServerConnection to wrap. */
      using ServerConnection = C;

      using Channel = typename ServerConnection::Channel;

      /**
       * Constructs a ToPythonServerConnection in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonServerConnection(Args&&... args);

      ~ToPythonServerConnection();

      /** Returns a reference to the underlying server connection. */
      ServerConnection& get();

      /** Returns a reference to the underlying server connection. */
      const ServerConnection& get() const;

      std::unique_ptr<Channel> accept();
      void close();

    private:
      boost::optional<ServerConnection> m_connection;

      ToPythonServerConnection(const ToPythonServerConnection&) = delete;
      ToPythonServerConnection& operator =(
        const ToPythonServerConnection&) = delete;
  };

  template<typename ServerConnection>
  ToPythonServerConnection(ServerConnection&&) ->
    ToPythonServerConnection<std::remove_cvref_t<ServerConnection>>;

  template<IsServerConnection C>
  template<typename... Args>
  ToPythonServerConnection<C>::ToPythonServerConnection(Args&&... args)
    : m_connection((pybind11::gil_scoped_release(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsServerConnection C>
  ToPythonServerConnection<C>::~ToPythonServerConnection() {
    auto release = pybind11::gil_scoped_release();
    m_connection.reset();
  }

  template<IsServerConnection C>
  typename ToPythonServerConnection<C>::ServerConnection&
      ToPythonServerConnection<C>::get() {
    return *m_connection;
  }

  template<IsServerConnection C>
  const typename ToPythonServerConnection<C>::ServerConnection&
      ToPythonServerConnection<C>::get() const {
    return *m_connection;
  }

  template<IsServerConnection C>
  std::unique_ptr<typename ToPythonServerConnection<C>::Channel>
      ToPythonServerConnection<C>::accept() {
    auto release = pybind11::gil_scoped_release();
    return m_connection->accept();
  }

  template<IsServerConnection C>
  void ToPythonServerConnection<C>::close() {
    auto release = pybind11::gil_scoped_release();
    m_connection->close();
  }
}

#endif
