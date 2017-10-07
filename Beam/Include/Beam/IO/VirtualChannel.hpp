#ifndef BEAM_VIRTUALCHANNEL_HPP
#define BEAM_VIRTUALCHANNEL_HPP
#include <boost/noncopyable.hpp>
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

  /*! \class VirtualChannel
      \brief Provides a pure virtual interface to a Channel.
   */
  class VirtualChannel : private boost::noncopyable {
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

      //! Constructs a VirtualChannel.
      VirtualChannel() = default;
  };

  /*! \class WrapperChannel
      \brief Wraps a Channel providing it with a virtual interface.
      \tparam ChannelType The type of Channel to wrap.
   */
  template<typename ChannelType>
  class WrapperVirtualChannel : public VirtualChannel {
    public:

      //! The Channel to wrap.
      using Channel = GetTryDereferenceType<ChannelType>;

      using Identifier = typename Channel::Identifier;

      using Connection = typename Channel::Connection;

      using Reader = typename Channel::Reader;

      using Writer = typename Channel::Writer;

      //! Constructs a WrapperChannel.
      /*!
        \param channel The Channel to wrap.
      */
      template<typename ChannelForward>
      WrapperVirtualChannel(ChannelForward&& channel);

      virtual ~WrapperVirtualChannel() override = default;

      virtual const VirtualChannelIdentifier& GetIdentifier() const override;

      virtual VirtualConnection& GetConnection() override;

      virtual VirtualReader& GetReader() override;

      virtual VirtualWriter& GetWriter() override;

    private:
      GetOptionalLocalPtr<ChannelType> m_channel;
      std::unique_ptr<VirtualChannelIdentifier> m_identifier;
      std::unique_ptr<VirtualConnection> m_connection;
      std::unique_ptr<VirtualReader> m_reader;
      std::unique_ptr<VirtualWriter> m_writer;
  };

  //! Wraps a Channel into a VirtualChannel.
  /*!
    \param channel The Channel to wrap.
  */
  template<typename Channel>
  std::unique_ptr<VirtualChannel> MakeVirtualChannel(Channel&& channel) {
    return std::make_unique<WrapperVirtualChannel<std::decay_t<Channel>>>(
      std::forward<Channel>(channel));
  }

  template<typename ChannelType>
  template<typename ChannelForward>
  WrapperVirtualChannel<ChannelType>::WrapperVirtualChannel(
      ChannelForward&& channel)
      : m_channel{std::forward<ChannelForward>(channel)},
        m_identifier{MakeVirtualChannelIdentifier(&m_channel->GetIdentifier())},
        m_connection{MakeVirtualConnection(&m_channel->GetConnection())},
        m_reader{MakeVirtualReader(&m_channel->GetReader())},
        m_writer{MakeVirtualWriter(&m_channel->GetWriter())} {}

  template<typename ChannelType>
  const VirtualChannelIdentifier& WrapperVirtualChannel<ChannelType>::
      GetIdentifier() const {
    return *m_identifier;
  }

  template<typename ChannelType>
  VirtualConnection& WrapperVirtualChannel<ChannelType>::GetConnection() {
    return *m_connection;
  }

  template<typename ChannelType>
  VirtualReader& WrapperVirtualChannel<ChannelType>::GetReader() {
    return *m_reader;
  }

  template<typename ChannelType>
  VirtualWriter& WrapperVirtualChannel<ChannelType>::GetWriter() {
    return *m_writer;
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualChannel,
    IO::Channel<IO::VirtualChannelIdentifier, IO::VirtualConnection,
    IO::VirtualReader, IO::VirtualWriter>> : std::true_type {};
}

#endif
