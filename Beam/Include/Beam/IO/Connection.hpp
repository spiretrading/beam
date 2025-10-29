#ifndef BEAM_CONNECTION_HPP
#define BEAM_CONNECTION_HPP
#include <concepts>
#include <memory>
#include <utility>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept satisfied by types that implement the Connection interface. */
  template<typename T>
  concept IsConnection = requires(T& t) {
    { t.close() } -> std::same_as<void>;
  };

  /** Specifies a connection based IO resource. */
  class Connection {
    public:

      /**
       * Constructs a Connection of a specified type using emplacement.
       * @tparam T The type of connection to emplace.
       * @param args The arguments to pass to the emplaced connection.
       */
      template<IsConnection T, typename... Args>
      explicit Connection(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Connection by referencing an existing connection.
       * @param connection The connection to reference.
       */
      template<DisableCopy<Connection> T> requires
        IsConnection<dereference_t<T>>
      Connection(T&& connection);

      Connection(const Connection&) = default;

      /** Closes the Connection. */
      void close();

    private:
      struct VirtualConnection {
        virtual ~VirtualConnection() = default;

        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedConnection final : VirtualConnection {
        using Connection = C;
        local_ptr_t<Connection> m_connection;

        template<typename... Args>
        WrappedConnection(Args&&... args);

        void close() override;
      };
      VirtualPtr<VirtualConnection> m_connection;
  };

  template<IsConnection T, typename... Args>
  Connection::Connection(std::in_place_type_t<T>, Args&&... args)
    : m_connection(
        make_virtual_ptr<WrappedConnection<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Connection> T> requires IsConnection<dereference_t<T>>
  Connection::Connection(T&& connection)
    : m_connection(make_virtual_ptr<WrappedConnection<std::remove_cvref_t<T>>>(
        std::forward<T>(connection))) {}

  inline void Connection::close() {
    m_connection->close();
  }

  template<typename C>
  template<typename... Args>
  Connection::WrappedConnection<C>::WrappedConnection(Args&&... args)
    : m_connection(std::forward<Args>(args)...) {}

  template<typename C>
  void Connection::WrappedConnection<C>::close() {
    m_connection->close();
  }
}

#endif
