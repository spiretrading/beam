#ifndef BEAM_CONNECTION_BOX_HPP
#define BEAM_CONNECTION_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface over an arbitrary Connection object. */
  class ConnectionBox {
    public:

      /**
       * Constructs a ConnectionBox of a specified type using emplacement.
       * @param <T> The type of connection to emplace.
       * @param args The arguments to pass to the emplaced connection.
       */
      template<typename T, typename... Args>
      explicit ConnectionBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ConnectionBox by copying an existing connection.
       * @param connection The connection to copy.
       */
      template<typename Connection>
      explicit ConnectionBox(Connection connection);

      explicit ConnectionBox(ConnectionBox* connection);

      explicit ConnectionBox(const std::shared_ptr<ConnectionBox>& connection);

      explicit ConnectionBox(const std::unique_ptr<ConnectionBox>& connection);

      void Close();

    protected:

      /** Specifies a pure virtual interface over a Connection. */
      class VirtualConnection {
        public:
          virtual ~VirtualConnection() = default;
          virtual void Close() = 0;
      };

      /**
       * Constructs a ConnectionBox using an existing VirtualConnection.
       * @param connection The existing connection to box.
       */
      ConnectionBox(std::shared_ptr<VirtualConnection> connection);

    private:
      template<typename C>
      struct WrappedConnection final : VirtualConnection {
        using Connection = C;
        GetOptionalLocalPtr<Connection> m_connection;

        template<typename... Args>
        WrappedConnection(Args&&... args);
        void Close() override;
      };
      std::shared_ptr<VirtualConnection> m_connection;
  };

  template<typename T, typename... Args>
  ConnectionBox::ConnectionBox(std::in_place_type_t<T>, Args&&... args)
    : m_connection(std::make_shared<WrappedConnection<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Connection>
  ConnectionBox::ConnectionBox(Connection connection)
    : ConnectionBox(std::in_place_type<Connection>,
        std::move<Connection>(connection)) {}

  inline ConnectionBox::ConnectionBox(ConnectionBox* connection)
    : ConnectionBox(*connection) {}

  inline ConnectionBox::ConnectionBox(
    const std::shared_ptr<ConnectionBox>& connection)
    : ConnectionBox(*connection) {}

  inline ConnectionBox::ConnectionBox(
    const std::unique_ptr<ConnectionBox>& connection)
    : ConnectionBox(*connection) {}

  inline void ConnectionBox::Close() {
    m_connection->Close();
  }

  inline ConnectionBox::ConnectionBox(
    std::shared_ptr<VirtualConnection> connection)
    : m_connection(std::move(connection)) {}

  template<typename C>
  template<typename... Args>
  ConnectionBox::WrappedConnection<C>::WrappedConnection(Args&&... args)
    : m_connection(std::forward<Args>(args)...) {}

  template<typename C>
  void ConnectionBox::WrappedConnection<C>::Close() {
    return m_connection->Close();
  }
}

  template<>
  struct ImplementsConcept<IO::ConnectionBox, IO::Connection> :
    std::true_type {};
}

#endif
