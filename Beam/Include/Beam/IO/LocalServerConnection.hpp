#ifndef BEAM_LOCAL_SERVER_CONNECTION_HPP
#define BEAM_LOCAL_SERVER_CONNECTION_HPP
#include <memory>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {
namespace IO {
namespace Details {
  template<typename Buffer>
  void Connect(LocalServerConnection<Buffer>& server,
    LocalClientChannel<Buffer>& client, const std::string&);
}

  /**
   * Implements a local ServerConnection.
   * @param <B> The type of Buffer the server's Channels read/write to.
   */
  template<typename B>
  class LocalServerConnection {
    public:

      /** The type of Buffer used by the Channel's Readers and Writers. */
      using Buffer = B;

      using Channel = LocalServerChannel<Buffer>;

      /** Constructs a LocalServerConnection. */
      LocalServerConnection() = default;

      ~LocalServerConnection();

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      template<typename Buffer>
      friend void Details::Connect(IO::LocalServerConnection<Buffer>&,
        IO::LocalClientChannel<Buffer>&, const std::string&);
      struct PendingChannelEntry {
        LocalClientChannel<Buffer>* m_client;
        std::string m_name;
        Routines::Eval<void> m_result;
      };
      Queue<PendingChannelEntry*> m_pendingChannels;

      LocalServerConnection(const LocalServerConnection&) = delete;
      LocalServerConnection& operator =(const LocalServerConnection&) = delete;
      void Connect(LocalClientChannel<Buffer>& client, const std::string& name);
  };

  template<typename B>
  LocalServerConnection<B>::~LocalServerConnection() {
    Close();
  }

  template<typename B>
  std::unique_ptr<typename LocalServerConnection<B>::Channel>
      LocalServerConnection<B>::Accept() {
    auto entry = [&] {
      try {
        return m_pendingChannels.Pop();
      } catch(const std::exception&) {
        BOOST_THROW_EXCEPTION(EndOfFileException());
      }
    }();
    auto serverReader = std::make_unique<PipedReader<Buffer>>();
    auto clientWriter = std::make_shared<PipedWriter<Buffer>>(
      Ref(*serverReader));
    auto clientReader = std::make_unique<PipedReader<Buffer>>();
    auto serverWriter = std::make_shared<PipedWriter<Buffer>>(
      Ref(*clientReader));
    auto channel = std::make_unique<Channel>(entry->m_name,
      std::move(serverReader), serverWriter, clientWriter);
    entry->m_client->m_reader = std::move(clientReader);
    entry->m_client->m_writer = clientWriter;
    entry->m_client->m_connection.emplace(std::move(clientWriter),
      std::move(serverWriter));
    entry->m_result.SetResult();
    return channel;
  }

  template<typename B>
  void LocalServerConnection<B>::Close() {
    m_pendingChannels.Break(ConnectException("Server unavailable."));
    while(auto entry = m_pendingChannels.TryPop()) {
      (*entry)->m_result.SetException(ConnectException("Server unavailable."));
    }
  }

  template<typename B>
  void LocalServerConnection<B>::Connect(LocalClientChannel<Buffer>& client,
      const std::string& name) {
    auto result = Routines::Async<void>();
    auto entry = PendingChannelEntry{&client, name, result.GetEval()};
    m_pendingChannels.Push(&entry);
    result.Get();
  }

namespace Details {
  template<typename Buffer>
  void Connect(LocalServerConnection<Buffer>& server,
      LocalClientChannel<Buffer>& client, const std::string& name) {
    server.Connect(client, name);
  }
}
}

  template<typename B>
  struct ImplementsConcept<IO::LocalServerConnection<B>,
    IO::ServerConnection<typename IO::LocalServerConnection<B>::Channel>> :
    std::true_type {};
}

#endif
