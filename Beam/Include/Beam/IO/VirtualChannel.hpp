#ifndef BEAM_VIRTUAL_CHANNEL_HPP
#define BEAM_VIRTUAL_CHANNEL_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/VirtualChannelIdentifier.hpp"
#include "Beam/IO/VirtualConnection.hpp"
#include "Beam/IO/VirtualReader.hpp"
#include "Beam/IO/VirtualWriter.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /** Provides a pure virtual interface to a Channel. */
  class VirtualChannel {
    public:
      using Identifier = VirtualChannelIdentifier;
      using Connection = VirtualConnection;
      using Reader = VirtualReader;
      using Writer = VirtualWriter;

      virtual ~VirtualChannel() = default;

      virtual const Identifier& GetIdentifier() const = 0;

      virtual Connection& GetConnection() = 0;

      virtual Reader& GetReader() = 0;

      virtual Writer& GetWriter() = 0;

    protected:

      /** Constructs a VirtualChannel. */
      VirtualChannel() = default;

    private:
      VirtualChannel(const VirtualChannel&) = delete;
      VirtualChannel& operator =(const VirtualChannel&) = delete;
  };

  /**
   * Wraps a Channel providing it with a virtual interface.
   * @param <C> The type of Channel to wrap.
   */
  template<typename C>
  class WrapperVirtualChannel : public VirtualChannel {
    public:

      /** The Channel to wrap. */
      using Channel = GetTryDereferenceType<C>;
      using Identifier = typename Channel::Identifier;
      using Connection = typename Channel::Connection;
      using Reader = typename Channel::Reader;
      using Writer = typename Channel::Writer;

      /**
       * Constructs a WrapperChannel.
       * @param channel The Channel to wrap.
       */
      template<typename CF>
      WrapperVirtualChannel(CF&& channel);

      const VirtualChannelIdentifier& GetIdentifier() const override;

      VirtualConnection& GetConnection() override;

      VirtualReader& GetReader() override;

      VirtualWriter& GetWriter() override;

    private:
      GetOptionalLocalPtr<C> m_channel;
      std::unique_ptr<VirtualChannelIdentifier> m_identifier;
      std::unique_ptr<VirtualConnection> m_connection;
      std::unique_ptr<VirtualReader> m_reader;
      std::unique_ptr<VirtualWriter> m_writer;
  };

  /**
   * Wraps a Channel into a VirtualChannel.
   * @param channel The Channel to wrap.
   */
  template<typename Channel>
  std::unique_ptr<VirtualChannel> MakeVirtualChannel(Channel&& channel) {
    return std::make_unique<WrapperVirtualChannel<std::decay_t<Channel>>>(
      std::forward<Channel>(channel));
  }

  template<typename C>
  template<typename CF>
  WrapperVirtualChannel<C>::WrapperVirtualChannel(CF&& channel)
    : m_channel(std::forward<CF>(channel)),
      m_identifier(MakeVirtualChannelIdentifier(&m_channel->GetIdentifier())),
      m_connection(MakeVirtualConnection(&m_channel->GetConnection())),
      m_reader(MakeVirtualReader(&m_channel->GetReader())),
      m_writer(MakeVirtualWriter(&m_channel->GetWriter())) {}

  template<typename C>
  const VirtualChannelIdentifier&
      WrapperVirtualChannel<C>::GetIdentifier() const {
    return *m_identifier;
  }

  template<typename C>
  VirtualConnection& WrapperVirtualChannel<C>::GetConnection() {
    return *m_connection;
  }

  template<typename C>
  VirtualReader& WrapperVirtualChannel<C>::GetReader() {
    return *m_reader;
  }

  template<typename C>
  VirtualWriter& WrapperVirtualChannel<C>::GetWriter() {
    return *m_writer;
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualChannel,
    IO::Channel<IO::VirtualChannelIdentifier, IO::VirtualConnection,
    IO::VirtualReader, IO::VirtualWriter>> : std::true_type {};
}

#endif
