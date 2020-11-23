#ifndef BEAM_SERVER_CONNECTION_BOX_HPP
#define BEAM_SERVER_CONNECTION_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/IO/ConnectionBox.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/ServerConnection.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface over an arbitrary ServerConnection object. */
  class ServerConnectionBox : public ConnectionBox {
    public:
      using Channel = ChannelBox;

      /**
       * Constructs a ServerConnectionBox of a specified type using emplacement.
       * @param <T> The type of server connection to emplace.
       * @param args The arguments to pass to the emplaced server connection.
       */
      template<typename T, typename... Args>
      explicit ServerConnectionBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ServerConnectionBox by copying an existing server
       * connection.
       * @param connection The server connection to copy.
       */
      template<typename ServerConnection>
      explicit ServerConnectionBox(ServerConnection connection);

      explicit ServerConnectionBox(ServerConnectionBox* connection);

      explicit ServerConnectionBox(
        const std::shared_ptr<ServerConnectionBox>& connection);

      explicit ServerConnectionBox(
        const std::unique_ptr<ServerConnectionBox>& connection);

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      struct VirtualServerConnection : public ConnectionBox::VirtualConnection {
        virtual std::unique_ptr<Channel> Accept() = 0;
      };
      template<typename C>
      struct WrappedServerConnection final : VirtualServerConnection {
        using ServerConnection = C;
        GetOptionalLocalPtr<ServerConnection> m_connection;

        template<typename... Args>
        WrappedServerConnection(Args&&... args);
        std::unique_ptr<Channel> Accept() override;
        void Close() override;
      };
      std::shared_ptr<VirtualServerConnection> m_connection;
  };

  template<typename T, typename... Args>
  ServerConnectionBox::ServerConnectionBox(std::in_place_type_t<T>,
    Args&&... args)
    : m_connection(std::make_shared<WrappedServerConnection<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ServerConnection>
  ServerConnectionBox::ServerConnectionBox(ServerConnection connection)
    : ServerConnectionBox(std::in_place_type<ServerConnection>,
        std::move(connection)) {}

  inline ServerConnectionBox::ServerConnectionBox(
    ServerConnectionBox* connection)
    : ServerConnectionBox(*connection) {}

  inline ServerConnectionBox::ServerConnectionBox(
    const std::shared_ptr<ServerConnectionBox>& connection)
    : ServerConnectionBox(*connection) {}

  inline ServerConnectionBox::ServerConnectionBox(
    const std::unique_ptr<ServerConnectionBox>& connection)
    : ServerConnectionBox(*connection) {}

  inline std::unique_ptr<ServerConnectionBox::Channel>
      ServerConnectionBox::Accept() {
    return m_connection->Accept();
  }

  inline void ServerConnectionBox::Close() {
    m_connection->Close();
  }

  template<typename C>
  template<typename... Args>
  ServerConnectionBox::WrappedServerConnection<C>::WrappedServerConnection(
    Args&&... args)
    : m_connection(std::forward<Args>(args)...) {}

  template<typename C>
  std::unique_ptr<ServerConnectionBox::Channel>
      ServerConnectionBox::WrappedServerConnection<C>::Accept() {
    return std::make_unique<ChannelBox>(m_connection->Accept());
  }

  template<typename C>
  void ServerConnectionBox::WrappedServerConnection<C>::Close() {
    return m_connection->Close();
  }
}

  template<>
  struct ImplementsConcept<IO::ServerConnectionBox,
    IO::ServerConnection<IO::ServerConnectionBox::Channel>> : std::true_type {};
}

#endif
