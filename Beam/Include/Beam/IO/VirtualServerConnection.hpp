#ifndef BEAM_VIRTUAL_SERVER_CONNECTION_HPP
#define BEAM_VIRTUAL_SERVER_CONNECTION_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/IO/VirtualConnection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a pure virtual interface to a ServerConnection. */
  class VirtualServerConnection : public VirtualConnection {
    public:
      using Channel = VirtualChannel;

      virtual std::unique_ptr<Channel> Accept() = 0;

    protected:

      /** Constructs a VirtualServerConnection. */
      VirtualServerConnection() = default;

    private:
      VirtualServerConnection(const VirtualServerConnection&) = delete;
      VirtualServerConnection& operator =(
        const VirtualServerConnection&) = delete;
  };

  /**
   * Wraps a ServerConnection providing it with a virtual interface.
   * @param <C> The type of Connection to wrap.
   */
  template<typename C>
  class WrapperServerConnection : public VirtualServerConnection {
    public:

      /** The ServerConnection to wrap. */
      using ServerConnection = GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperServerConnection.
       * @param connection The ServerConnection to wrap.
       */
      template<typename CF>
      WrapperServerConnection(CF&& connection);

      std::unique_ptr<Channel> Accept() override;

      void Close() override;

    private:
      GetOptionalLocalPtr<C> m_connection;
  };

  /**
   * Wraps a ServerConnection into a VirtualServerConnection.
   * @param connection The ServerConnection to wrap.
   */
  template<typename ServerConnection>
  std::unique_ptr<VirtualServerConnection> MakeVirtualServerConnection(
      ServerConnection&& connection) {
    return std::make_unique<WrapperServerConnection<
      std::decay_t<ServerConnection>>>(
      std::forward<ServerConnection>(connection));
  }

  template<typename C>
  template<typename CF>
  WrapperServerConnection<C>::WrapperServerConnection(CF&& connection)
    : m_connection(std::forward<CF>(connection)) {}

  template<typename C>
  std::unique_ptr<VirtualServerConnection::Channel>
      WrapperServerConnection<C>::Accept() {
    return MakeVirtualChannel(m_connection->Accept());
  }

  template<typename C>
  void WrapperServerConnection<C>::Close() {
    return m_connection->Close();
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualServerConnection,
    IO::ServerConnection<IO::VirtualServerConnection::Channel>> :
    std::true_type {};
}

#endif
