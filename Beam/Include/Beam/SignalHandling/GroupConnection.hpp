#ifndef BEAM_GROUPCONNECTION_HPP
#define BEAM_GROUPCONNECTION_HPP
#include <vector>
#include <boost/signals2.hpp>
#include "Beam/SignalHandling/SignalHandling.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \class GroupConnection
      \brief Aggregates individual connections together.
   */
  class GroupConnection {
    public:

      //! Constructs a GroupConnection.
      GroupConnection() = default;

      //! Constructs a GroupConnection from a list of connections.
      /*!
        \param connections The list of connections to group together.
      */
      GroupConnection(
        const std::vector<boost::signals2::connection>& connections);

      //! Constructs a GroupConnection from a single connection.
      /*!
        \param connection The individual connection in the group.
      */
      GroupConnection(const boost::signals2::connection& connection);

      //! Constructs a GroupConnection from a collection of connection.
      /*!
        \param connection0 A connection in the group.
        \param connection1 A connection in the group.
      */
      GroupConnection(const boost::signals2::connection& connection0,
        const boost::signals2::connection& connection1);

      //! Constructs a GroupConnection from a collection of connection.
      /*!
        \param connection0 A connection in the group.
        \param connection1 A connection in the group.
        \param connection2 A connection in the group.
      */
      GroupConnection(const boost::signals2::connection& connection0,
        const boost::signals2::connection& connection1,
        const boost::signals2::connection& connection2);

      //! Constructs a GroupConnection from a collection of connection.
      /*!
        \param connection0 A connection in the group.
        \param connection1 A connection in the group.
        \param connection2 A connection in the group.
        \param connection3 A connection in the group.
      */
      GroupConnection(const boost::signals2::connection& connection0,
        const boost::signals2::connection& connection1,
        const boost::signals2::connection& connection2,
        const boost::signals2::connection& connection3);

      //! Returns the list of connections in this group.
      const std::vector<boost::signals2::connection>& GetConnections() const;

      //! Disconnects all connections.
      void Disconnect();

    private:
      std::vector<boost::signals2::connection> m_connections;
  };

  inline GroupConnection::GroupConnection(
      const std::vector<boost::signals2::connection>& connections)
      : m_connections(connections) {}

  inline GroupConnection::GroupConnection(
      const boost::signals2::connection& connection) {
    m_connections.push_back(connection);
  }

  inline GroupConnection::GroupConnection(
      const boost::signals2::connection& connection0,
      const boost::signals2::connection& connection1) {
    m_connections.push_back(connection0);
    m_connections.push_back(connection1);
  }

  inline GroupConnection::GroupConnection(
      const boost::signals2::connection& connection0,
      const boost::signals2::connection& connection1,
      const boost::signals2::connection& connection2) {
    m_connections.push_back(connection0);
    m_connections.push_back(connection1);
    m_connections.push_back(connection2);
  }

  inline GroupConnection::GroupConnection(
      const boost::signals2::connection& connection0,
      const boost::signals2::connection& connection1,
      const boost::signals2::connection& connection2,
      const boost::signals2::connection& connection3) {
    m_connections.push_back(connection0);
    m_connections.push_back(connection1);
    m_connections.push_back(connection2);
    m_connections.push_back(connection3);
  }

  inline const std::vector<boost::signals2::connection>&
      GroupConnection::GetConnections() const {
    return m_connections;
  }

  inline void GroupConnection::Disconnect() {
    for(auto& connection : m_connections) {
      connection.disconnect();
    }
  }
}
}

#endif
