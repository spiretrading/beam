#ifndef BEAM_LOCALSERVERCONNECTION_HPP
#define BEAM_LOCALSERVERCONNECTION_HPP
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {
namespace IO {

  /*! \class LocalServerConnection
      \brief Implements a local ServerConnection.
      \tparam BufferType The type of Buffer the server's Channels read/write to.
   */
  template<typename BufferType>
  class LocalServerConnection : private boost::noncopyable {
    public:

      //! The type of Buffer used by the Channel's Readers and Writers.
      using Buffer = BufferType;

      using Channel = IO::LocalServerChannel<Buffer>;

      //! Constructs a LocalServerConnection.
      LocalServerConnection();

      ~LocalServerConnection();

      std::unique_ptr<Channel> Accept();

      void Open();

      void Close();

    private:
      friend class IO::LocalServerChannel<Buffer>;
      using ClientToServerChannels = std::unordered_map<
        LocalClientChannel<Buffer>*, std::unique_ptr<Channel>>;
      struct PendingChannelEntry {
        LocalClientChannel<Buffer>* m_clientChannel;
        Routines::Eval<void> m_result;

        PendingChannelEntry(Ref<LocalClientChannel<Buffer>> clientChannel,
          Routines::Eval<void>&& result);
      };
      friend class LocalClientChannel<Buffer>;
      friend class LocalConnection<Buffer>;
      Threading::Sync<ClientToServerChannels> m_clientToServerChannels;
      std::shared_ptr<Queue<PendingChannelEntry*>> m_pendingChannels;

      void AddChannel(Ref<LocalClientChannel<Buffer>> clientChannel,
        std::unique_ptr<Channel> channel);
  };

  template<typename BufferType>
  LocalServerConnection<BufferType>::PendingChannelEntry::PendingChannelEntry(
      Ref<LocalClientChannel<Buffer>> clientChannel,
      Routines::Eval<void>&& result)
      : m_clientChannel{clientChannel.Get()},
        m_result{std::move(result)} {}

  template<typename BufferType>
  LocalServerConnection<BufferType>::LocalServerConnection()
      : m_pendingChannels{std::make_shared<Queue<PendingChannelEntry*>>()} {}

  template<typename BufferType>
  LocalServerConnection<BufferType>::~LocalServerConnection() {
    Close();
  }

  template<typename BufferType>
  std::unique_ptr<typename LocalServerConnection<BufferType>::Channel>
      LocalServerConnection<BufferType>::Accept() {
    PendingChannelEntry* entry;
    try {
      entry = m_pendingChannels->Pop();
    } catch(const std::exception&) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    auto channel = Threading::With(m_clientToServerChannels,
      [&] (ClientToServerChannels& clientToServerChannels) {
        auto channelIterator = clientToServerChannels.find(
          entry->m_clientChannel);
        if(channelIterator == clientToServerChannels.end()) {
          return std::unique_ptr<Channel>{};
        }
        auto channel = std::move(channelIterator->second);
        clientToServerChannels.erase(channelIterator);
        return channel;
      });
    if(channel == nullptr) {
      return Accept();
    }
    channel->m_connection.m_isOpen = true;
    entry->m_result.SetResult();
    return channel;
  }

  template<typename BufferType>
  void LocalServerConnection<BufferType>::Open() {}

  template<typename BufferType>
  void LocalServerConnection<BufferType>::Close() {
    m_pendingChannels->Break(NotConnectedException());
    while(auto entry = m_pendingChannels->TryPop()) {
      (*entry)->m_result.SetException(ConnectException("Server unavailable."));
    }
  }

  template<typename BufferType>
  void LocalServerConnection<BufferType>::AddChannel(
      Ref<LocalClientChannel<Buffer>> clientChannel,
      std::unique_ptr<Channel> channel) {
    Threading::With(m_clientToServerChannels,
      [&] (ClientToServerChannels& clientToServerChannels) {
        clientToServerChannels.insert(std::make_pair(clientChannel.Get(),
          std::move(channel)));
      });
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::LocalServerConnection<BufferType>,
    IO::ServerConnection<typename IO::LocalServerConnection<
    BufferType>::Channel>> : std::true_type {};
}

#endif
