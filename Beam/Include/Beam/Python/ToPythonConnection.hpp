#ifndef BEAM_TO_PYTHON_CONNECTION_HPP
#define BEAM_TO_PYTHON_CONNECTION_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/IO/VirtualConnection.hpp"

namespace Beam::IO {

  /**
   * Wraps a Connection for use with Python.
   * @param <C> The type of Connection to wrap.
   */
  template<typename C>
  class ToPythonConnection : public VirtualConnection {
    public:

      /** The type of Connection to wrap. */
      using Connection = C;

      /**
       * Constructs a ToPythonConnection.
       * @param connection The Connection to wrap.
       */
      ToPythonConnection(std::unique_ptr<Connection> connection);

      ~ToPythonConnection() override;

      void Close() override;

    private:
      std::unique_ptr<Connection> m_connection;
  };

  /**
   * Makes a ToPythonConnection.
   * @param connection The Connection to wrap.
   */
  template<typename Connection>
  auto MakeToPythonConnection(std::unique_ptr<Connection> connection) {
    return std::make_unique<ToPythonConnection<Connection>>(
      std::move(connection));
  }

  template<typename C>
  ToPythonConnection<C>::ToPythonConnection(
    std::unique_ptr<Connection> connection)
    : m_connection(std::move(connection)) {}

  template<typename C>
  ToPythonConnection<C>::~ToPythonConnection() {
    Close();
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
