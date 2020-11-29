#ifndef BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#define BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::IO {

  /**
   * Wraps a ServerConnection for use with Python.
   * @param <C> The type of ServerConnection to wrap.
   */
  template<typename C>
  class ToPythonServerConnection {
    public:

      /** The type of ServerConnection to wrap. */
      using ServerConnection = C;
      using Channel = ChannelBox;

      /**
       * Constructs a ToPythonServerConnection in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonServerConnection(Args&&... args);

      ~ToPythonServerConnection();

      ToPythonServerConnection(ToPythonServerConnection&&) = default;

      /** Returns the wrapped ServerConnection. */
      const ServerConnection& GetConnection() const;

      /** Returns the wrapped ServerConnection. */
      ServerConnection& GetConnection();

      std::unique_ptr<Channel> Accept();

      void Close();

      ToPythonServerConnection& operator =(
        ToPythonServerConnection&&) = default;

    private:
      boost::optional<ServerConnection> m_connection;

      ToPythonServerConnection(const ToPythonServerConnection&) = delete;
      ToPythonServerConnection& operator =(
        const ToPythonServerConnection&) = delete;
  };

  template<typename ServerConnection>
  ToPythonServerConnection(ServerConnection&&) ->
    ToPythonServerConnection<std::decay_t<ServerConnection>>;

  template<typename C>
  template<typename... Args>
  ToPythonServerConnection<C>::ToPythonServerConnection(Args&&... args)
    : m_connection((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonServerConnection<C>::~ToPythonServerConnection() {
    auto release = Python::GilRelease();
    m_connection.reset();
  }

  template<typename C>
  const typename ToPythonServerConnection<C>::ServerConnection&
      ToPythonServerConnection<C>::GetConnection() const {
    return *m_connection;
  }

  template<typename C>
  typename ToPythonServerConnection<C>::ServerConnection&
      ToPythonServerConnection<C>::GetConnection() {
    return *m_connection;
  }

  template<typename C>
  std::unique_ptr<typename ToPythonServerConnection<C>::Channel>
      ToPythonServerConnection<C>::Accept() {
    auto release = Python::GilRelease();
    return std::make_unique<Channel>(m_connection->Accept());
  }

  template<typename C>
  void ToPythonServerConnection<C>::Close() {
    auto release = Python::GilRelease();
    m_connection->Close();
  }
}

#endif
