#ifndef BEAM_CONNECTION_GROUP_HPP
#define BEAM_CONNECTION_GROUP_HPP
#include <unordered_map>
#include <vector>
#include <boost/signals2.hpp>

namespace Beam {

  /** Manages a group of connections. */
  class ConnectionGroup {
    public:

      /** Constructs an empty ConnectionGroup. */
      ConnectionGroup() = default;

      ~ConnectionGroup();

      /**
       * Adds a connection to manage.
       * @param connection The connection to manage.
       */
      void add(const boost::signals2::connection& connection);

      /**
       * Adds a connection associated with a publisher.
       * @param publisher The publisher to associate the connection with.
       * @param connection The connection to manage.
       */
      void add(
        const void* publisher, const boost::signals2::connection& connection);

      /**
       * Disconnects all connections associated with a publisher.
       * @param publisher The publisher to disconnect all connections from.
       */
      void disconnect(const void* publisher);

      /** Disconnects all connections. */
      void disconnect();

    private:
      std::unordered_map<
        const void*, std::vector<boost::signals2::scoped_connection>>
          m_connections;
  };

  inline ConnectionGroup::~ConnectionGroup() {
    disconnect();
  }

  inline void ConnectionGroup::add(
      const boost::signals2::connection& connection) {
    add(nullptr, connection);
  }

  inline void ConnectionGroup::add(
      const void* publisher, const boost::signals2::connection& connection) {
    m_connections[publisher].push_back(connection);
  }

  inline void ConnectionGroup::disconnect(const void* publisher) {
    m_connections.erase(publisher);
  }

  inline void ConnectionGroup::disconnect() {
    m_connections.clear();
  }
}

#endif
