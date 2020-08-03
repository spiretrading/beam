#ifndef BEAM_CONNECTIONGROUP_HPP
#define BEAM_CONNECTIONGROUP_HPP
#include <map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include "Beam/SignalHandling/GroupConnection.hpp"
#include "Beam/SignalHandling/SignalHandling.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \class ConnectionGroup
      \brief Manages a group of connections.
   */
  class ConnectionGroup : private boost::noncopyable {
    public:

      //! Constructs a ConnectionGroup.
      ConnectionGroup() = default;

      ~ConnectionGroup();

      //! Adds a connection to manage.
      /*!
        \param connection The connection to manage.
      */
      void AddConnection(const boost::signals2::connection& connection);

      //! Adds a SignalHandling::GroupConnection to manage.
      /*!
        \param connection The connection to manage.
      */
      void AddConnection(const SignalHandling::GroupConnection& connection);

      //! Adds a connection associated with a publisher.
      /*!
        \param publisher The publisher to associate the connection with.
        \param connection The connection to manage.
      */
      void AddConnection(const void* publisher,
        const boost::signals2::connection& connection);

      //! Adds a SignalHandling::GroupConnection associated with a publisher.
      /*!
        \param publisher The publisher to associate the connection with.
        \param connection The connection to manage.
      */
      void AddConnection(const void* publisher,
        const SignalHandling::GroupConnection& connection);

      //! Disconnects all connections associated with a publisher.
      /*!
        \param publisher The publisher to disconnect all connections from.
      */
      void Disconnect(const void* publisher);

      //! Disconnects all connections.
      void DisconnectAll();

    private:
      std::map<const void*,
        std::vector<std::unique_ptr<boost::signals2::scoped_connection>>>
        m_connections;
  };

  inline ConnectionGroup::~ConnectionGroup() {
    DisconnectAll();
  }

  inline void ConnectionGroup::AddConnection(
      const boost::signals2::connection& connection) {
    m_connections[nullptr].push_back(
      std::make_unique<boost::signals2::scoped_connection>(connection));
  }

  inline void ConnectionGroup::AddConnection(
      const SignalHandling::GroupConnection& connection) {
    for(auto& c : connection.GetConnections()) {
      AddConnection(c);
    }
  }

  inline void ConnectionGroup::AddConnection(const void* publisher,
      const boost::signals2::connection& connection) {
    m_connections[publisher].push_back(
      std::make_unique<boost::signals2::scoped_connection>(connection));
  }

  inline void ConnectionGroup::AddConnection(const void* publisher,
      const SignalHandling::GroupConnection& connection) {
    for(auto& c : connection.GetConnections()) {
      AddConnection(publisher, c);
    }
  }

  inline void ConnectionGroup::Disconnect(const void* publisher) {
    auto connectionIterator = m_connections.find(publisher);
    if(connectionIterator == m_connections.end()) {
      return;
    }
    m_connections.erase(connectionIterator);
  }

  inline void ConnectionGroup::DisconnectAll() {
    m_connections.clear();
  }
}
}

#endif
