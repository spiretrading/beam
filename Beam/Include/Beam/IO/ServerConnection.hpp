#ifndef BEAM_SERVER_CONNECTION_HPP
#define BEAM_SERVER_CONNECTION_HPP
#include <concepts>
#include <memory>
#include <utility>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Concept satisfied by types that implement the ServerConnection interface.
   */
  template<typename T>
  concept IsServerConnection = requires(T& t) {
    typename T::Channel;
    { t.accept() } -> std::same_as<std::unique_ptr<typename T::Channel>>;
  } && IsConnection<T> && IsChannel<typename T::Channel>;

  /** Interface for the server side of a Connection. */
  class ServerConnection {
    public:

      /** Defines the type of Channel accepted by this server. */
      using Channel = Beam::Channel;

      /**
       * Constructs a ServerConnection of a specified type using emplacement.
       * @tparam T The type of server connection to emplace.
       * @param args The arguments to pass to the emplaced server connection.
       */
      template<IsServerConnection T, typename... Args>
      explicit ServerConnection(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ServerConnection by referencing an existing server
       * connection.
       * @param connection The server connection to reference.
       */
      template<DisableCopy<ServerConnection> T> requires
        IsServerConnection<dereference_t<T>>
      ServerConnection(T&& connection);

      ServerConnection(const ServerConnection&) = default;

      /**
       * Accepts a new Channel.
       * @return The Channel that was accepted.
       */
      std::unique_ptr<Channel> accept();

      void close();

    private:
      struct VirtualServerConnection {
        virtual ~VirtualServerConnection() = default;

        virtual std::unique_ptr<Channel> accept() = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedServerConnection final : VirtualServerConnection {
        using Connection = C;
        local_ptr_t<Connection> m_connection;

        template<typename... Args>
        WrappedServerConnection(Args&&... args);

        std::unique_ptr<Channel> accept() override;
        void close() override;
      };
      VirtualPtr<VirtualServerConnection> m_connection;
  };

  template<IsServerConnection T, typename... Args>
  ServerConnection::ServerConnection(std::in_place_type_t<T>, Args&&... args)
    : m_connection(make_virtual_ptr<WrappedServerConnection<T>>(
        std::forward<Args>(args)...)) {}

  template<DisableCopy<ServerConnection> T> requires
    IsServerConnection<dereference_t<T>>
  ServerConnection::ServerConnection(T&& connection)
    : m_connection(make_virtual_ptr<WrappedServerConnection<
        std::remove_cvref_t<T>>>(std::forward<T>(connection))) {}

  inline std::unique_ptr<Channel> ServerConnection::accept() {
    return m_connection->accept();
  }

  inline void ServerConnection::close() {
    m_connection->close();
  }

  template<typename C>
  template<typename... Args>
  ServerConnection::WrappedServerConnection<C>::WrappedServerConnection(
    Args&&... args)
    : m_connection(std::forward<Args>(args)...) {}

  template<typename C>
  std::unique_ptr<Channel>
      ServerConnection::WrappedServerConnection<C>::accept() {
    if constexpr(std::is_same_v<
        typename dereference_t<Connection>::Channel, Beam::Channel>) {
      return m_connection->accept();
    } else {
      return std::make_unique<Channel>(m_connection->accept());
    }
  }

  template<typename C>
  void ServerConnection::WrappedServerConnection<C>::close() {
    m_connection->close();
  }
}

#endif
