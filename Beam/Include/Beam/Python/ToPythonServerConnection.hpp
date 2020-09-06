#ifndef BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#define BEAM_TO_PYTHON_SERVER_CONNECTION_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/IO/VirtualServerConnection.hpp"

namespace Beam::IO {

  /**
   * Wraps a ServerConnection for use with Python.
   * @param <C> The type of ServerConnection to wrap.
   */
  template<typename C>
  class ToPythonServerConnection final : public VirtualServerConnection {
    public:

      /** The type of ServerConnection to wrap. */
      using ServerConnection = C;

      /**
       * Constructs a ToPythonServerConnection.
       * @param connection The ServerConnection to wrap.
       */
      ToPythonServerConnection(std::unique_ptr<ServerConnection> connection);

      ~ToPythonServerConnection() override;

      std::unique_ptr<Channel> Accept() override;

      void Close() override;

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
  ToPythonServerConnection<C>::~ToPythonServerConnection() {
    Close();
    auto release = Python::GilRelease();
    m_connection.reset();
  }

  template<typename C>
  std::unique_ptr<typename ToPythonServerConnection<C>::Channel>
      ToPythonServerConnection<C>::Accept() {
    auto release = Python::GilRelease();
    return MakeVirtualChannel(m_connection->Accept());
  }

  template<typename C>
  void ToPythonServerConnection<C>::Close() {
    auto release = Python::GilRelease();
    m_connection->Close();
  }
}

#endif
