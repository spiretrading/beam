#ifndef BEAM_LOCAL_SERVER_CONNECTION_HPP
#define BEAM_LOCAL_SERVER_CONNECTION_HPP
#include <memory>
#include <boost/throw_exception.hpp>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {
namespace Details {
  void connect(LocalServerConnection&, LocalClientChannel&, const std::string&);
}

  /** Implements a local ServerConnection. */
  class LocalServerConnection {
    public:
      using Channel = LocalServerChannel;

      /** Constructs a LocalServerConnection. */
      LocalServerConnection() = default;

      ~LocalServerConnection();

      std::unique_ptr<Channel> accept();
      void close();

    private:
      friend void Details::connect(
        LocalServerConnection&, LocalClientChannel&, const std::string&);
      struct PendingChannelEntry {
        LocalClientChannel* m_client;
        std::string m_name;
        Eval<void> m_result;
      };
      Queue<PendingChannelEntry*> m_pending_channels;

      LocalServerConnection(const LocalServerConnection&) = delete;
      LocalServerConnection& operator =(const LocalServerConnection&) = delete;
      void connect(LocalClientChannel& client, const std::string& name);
  };

  inline LocalServerConnection::~LocalServerConnection() {
    close();
  }

  inline std::unique_ptr<LocalServerConnection::Channel>
      LocalServerConnection::accept() {
    auto entry = [&] {
      try {
        return m_pending_channels.pop();
      } catch(const std::exception&) {
        boost::throw_with_location(EndOfFileException());
      }
    }();
    auto server_reader = std::make_unique<PipedReader>();
    auto client_writer = std::make_shared<PipedWriter>(Ref(*server_reader));
    auto client_reader = std::make_unique<PipedReader>();
    auto server_writer = std::make_shared<PipedWriter>(Ref(*client_reader));
    auto channel = std::make_unique<Channel>(
      entry->m_name, std::move(server_reader), server_writer, client_writer);
    entry->m_client->m_reader = std::move(client_reader);
    entry->m_client->m_writer = client_writer;
    entry->m_client->m_connection.emplace(
      std::move(client_writer), std::move(server_writer));
    entry->m_result.set();
    return channel;
  }

  inline void LocalServerConnection::close() {
    m_pending_channels.close(ConnectException("Server unavailable."));
    while(auto entry = m_pending_channels.try_pop()) {
      (*entry)->m_result.set_exception(ConnectException("Server unavailable."));
    }
  }

  inline void LocalServerConnection::connect(
      LocalClientChannel& client, const std::string& name) {
    auto result = Async<void>();
    auto entry = PendingChannelEntry(&client, name, result.get_eval());
    m_pending_channels.push(&entry);
    result.get();
  }

namespace Details {
  inline void connect(LocalServerConnection& server, LocalClientChannel& client,
      const std::string& name) {
    server.connect(client, name);
  }
}
}

#endif
