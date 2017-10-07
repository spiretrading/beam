#ifndef BEAM_VIRTUALCONNECTION_HPP
#define BEAM_VIRTUALCONNECTION_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class VirtualConnection
      \brief Provides a pure virtual interface to a Connection.
   */
  class VirtualConnection : private boost::noncopyable {
    public:
      virtual ~VirtualConnection() = default;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualConnection.
      VirtualConnection() = default;
  };

  /*! \class WrapperConnection
      \brief Wraps a Connection providing it with a virtual interface.
      \tparam ConnectionType The type of Connection to wrap.
   */
  template<typename ConnectionType>
  class WrapperConnection : public VirtualConnection {
    public:

      //! The Connection to wrap.
      using Connection = GetTryDereferenceType<ConnectionType>;

      //! Constructs a WrapperConnection.
      /*!
        \param connection The Connection to wrap.
      */
      template<typename ConnectionForward>
      WrapperConnection(ConnectionForward&& connection);

      virtual ~WrapperConnection() override = default;

      //! Returns the Connection being wrapped.
      const Connection& GetConnection() const;

      //! Returns the Connection being wrapped.
      Connection& GetConnection();

      virtual void Open() override;

      virtual void Close() override;

    private:
      GetOptionalLocalPtr<ConnectionType> m_connection;
  };

  //! Wraps a Connection into a VirtualConnection.
  /*!
    \param connection The Connection to wrap.
  */
  template<typename Connection>
  std::unique_ptr<VirtualConnection> MakeVirtualConnection(
      Connection&& connection) {
    return std::make_unique<WrapperConnection<std::decay_t<Connection>>>(
      std::forward<Connection>(connection));
  }

  template<typename ConnectionType>
  template<typename ConnectionForward>
  WrapperConnection<ConnectionType>::WrapperConnection(
      ConnectionForward&& connection)
      : m_connection{std::forward<ConnectionForward>(connection)} {}

  template<typename ConnectionType>
  const typename WrapperConnection<ConnectionType>::Connection&
      WrapperConnection<ConnectionType>::GetConnection() const {
    return *m_connection;
  }

  template<typename ConnectionType>
  typename WrapperConnection<ConnectionType>::Connection&
      WrapperConnection<ConnectionType>::GetConnection() {
    return *m_connection; 
  }

  template<typename ConnectionType>
  void WrapperConnection<ConnectionType>::Open() {
    return m_connection->Open();
  }

  template<typename ConnectionType>
  void WrapperConnection<ConnectionType>::Close() {
    return m_connection->Close();
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualConnection,
    IO::Connection> : std::true_type {};
}

#endif
