#ifndef BEAM_TO_PYTHON_CONNECTION_HPP
#define BEAM_TO_PYTHON_CONNECTION_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Python/GilRelease.hpp"

namespace Beam::Python {

  /**
   * Wraps a Connection for use with Python.
   * @tparam C The type of Connection to wrap.
   */
  template<IsConnection C>
  class ToPythonConnection {
    public:

      /** The type of Connection to wrap. */
      using Connection = C;

      /**
       * Constructs a ToPythonConnection in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonConnection(Args&&... args);

      ~ToPythonConnection();

      /** Returns a reference to the underlying connection. */
      Connection& get();

      /** Returns a reference to the underlying connection. */
      const Connection& get() const;

      /** Returns a reference to the underlying connection. */
      Connection& operator *();

      /** Returns a reference to the underlying connection. */
      const Connection& operator *() const;

      /** Returns a pointer to the underlying connection. */
      Connection* operator ->();

      /** Returns a pointer to the underlying connection. */
      const Connection* operator ->() const;

      void close();

    private:
      boost::optional<Connection> m_connection;

      ToPythonConnection(const ToPythonConnection&) = delete;
      ToPythonConnection& operator =(const ToPythonConnection&) = delete;
  };

  template<typename Connection>
  ToPythonConnection(Connection&&) ->
    ToPythonConnection<std::remove_cvref_t<Connection>>;

  template<IsConnection C>
  template<typename... Args>
  ToPythonConnection<C>::ToPythonConnection(Args&&... args)
    : m_connection((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsConnection C>
  ToPythonConnection<C>::~ToPythonConnection() {
    auto release = GilRelease();
    m_connection.reset();
  }

  template<IsConnection C>
  typename ToPythonConnection<C>::Connection& ToPythonConnection<C>::get() {
    return *m_connection;
  }

  template<IsConnection C>
  const typename ToPythonConnection<C>::Connection&
      ToPythonConnection<C>::get() const {
    return *m_connection;
  }

  template<IsConnection C>
  typename ToPythonConnection<C>::Connection&
      ToPythonConnection<C>::operator *() {
    return *m_connection;
  }

  template<IsConnection C>
  const typename ToPythonConnection<C>::Connection&
      ToPythonConnection<C>::operator *() const {
    return *m_connection;
  }

  template<IsConnection C>
  typename ToPythonConnection<C>::Connection*
      ToPythonConnection<C>::operator ->() {
    return m_connection.get_ptr();
  }

  template<IsConnection C>
  const typename ToPythonConnection<C>::Connection*
      ToPythonConnection<C>::operator ->() const {
    return m_connection.get_ptr();
  }

  template<IsConnection C>
  void ToPythonConnection<C>::close() {
    auto release = GilRelease();
    m_connection->close();
  }
}

#endif
