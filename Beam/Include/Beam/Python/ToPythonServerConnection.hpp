#ifndef BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#define BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
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
       * Constructs a ToPythonServerConnection.
       * @param connection The ServerConnection to wrap.
       */
      ToPythonServerConnection(std::unique_ptr<ServerConnection> connection);

      /**
       * Constructs a ToPythonServerConnection in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonServerConnection(Args&&... args);

      ~ToPythonServerConnection();

      /** Returns the wrapped ServerConnection. */
      const ServerConnection& GetConnection() const;

      /** Returns the wrapped ServerConnection. */
      ServerConnection& GetConnection();

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      std::unique_ptr<ServerConnection> m_connection;
  };

  /**
   * Makes a ToPythonServerConnection.
   * @param connection The ServerConnection to wrap.
   */
  template<typename ServerConnection>
  auto MakeToPythonServerConnection(
      std::unique_ptr<ServerConnection> connection) {
    return std::make_unique<ToPythonServerConnection<ServerConnection>>(
      std::move(connection));
  }

  template<typename C>
  ToPythonServerConnection<C>::ToPythonServerConnection(
    std::unique_ptr<ServerConnection> connection)
    : m_connection(std::move(connection)) {}

  template<typename C>
  template<typename... Args>
  ToPythonServerConnection<C>::ToPythonServerConnection(Args&&... args)
    : ToPythonServerConnection(std::make_unique<ServerConnection>(
        std::forward<Args>(args)...)) {}

  template<typename C>
  ToPythonServerConnection<C>::~ToPythonServerConnection() {
    Close();
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
