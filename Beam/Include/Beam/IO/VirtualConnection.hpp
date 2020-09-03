#ifndef BEAM_VIRTUAL_CONNECTION_HPP
#define BEAM_VIRTUAL_CONNECTION_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a pure virtual interface to a Connection. */
  class VirtualConnection {
    public:
      virtual ~VirtualConnection() = default;

      virtual void Close() = 0;

    protected:

      /** Constructs a VirtualConnection. */
      VirtualConnection() = default;

    private:
      VirtualConnection(const VirtualConnection&) = delete;
      VirtualConnection& operator =(const VirtualConnection&) = delete;
  };

  /**
   * Wraps a Connection providing it with a virtual interface.
   * @param <C> The type of Connection to wrap.
   */
  template<typename C>
  class WrapperConnection : public VirtualConnection {
    public:

      /** The Connection to wrap. */
      using Connection = GetTryDereferenceType<C>;

      /**
       * Constructs a WrapperConnection.
       * @param connection The Connection to wrap.
       */
      template<typename CF>
      WrapperConnection(CF&& connection);

      void Close() override;

    private:
      GetOptionalLocalPtr<C> m_connection;
  };

  /**
   * Wraps a Connection into a VirtualConnection.
   * @param connection The Connection to wrap.
   */
  template<typename Connection>
  std::unique_ptr<VirtualConnection> MakeVirtualConnection(
      Connection&& connection) {
    return std::make_unique<WrapperConnection<std::decay_t<Connection>>>(
      std::forward<Connection>(connection));
  }

  template<typename C>
  template<typename CF>
  WrapperConnection<C>::WrapperConnection(CF&& connection)
    : m_connection(std::forward<CF>(connection)) {}

  template<typename C>
  void WrapperConnection<C>::Close() {
    m_connection->Close();
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualConnection, IO::Connection> :
    std::true_type {};
}

#endif
