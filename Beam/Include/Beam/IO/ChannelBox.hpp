#ifndef BEAM_CHANNEL_BOX_HPP
#define BEAM_CHANNEL_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/ChannelIdentifierBox.hpp"
#include "Beam/IO/ConnectionBox.hpp"
#include "Beam/IO/ReaderBox.hpp"
#include "Beam/IO/WriterBox.hpp"
#include "Beam/Pointers/Dereference.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface over an arbitrary Channel object. */
  class ChannelBox {
    public:
      using Identifier = ChannelIdentifierBox;
      using Connection = ConnectionBox;
      using Reader = ReaderBox;
      using Writer = WriterBox;

      /**
       * Constructs a ChannelBox of a specified type using emplacement.
       * @param <T> The type of channel to emplace.
       * @param args The arguments to pass to the emplaced channel.
       */
      template<typename T, typename... Args>
      explicit ChannelBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ChannelBox by copying an existing channel.
       * @param channel The channel to copy.
       */
      template<typename Channel>
      explicit ChannelBox(Channel channel);

      explicit ChannelBox(ChannelBox* channel);

      explicit ChannelBox(const std::shared_ptr<ChannelBox>& channel);

      explicit ChannelBox(const std::unique_ptr<ChannelBox>& channel);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      std::shared_ptr<void> m_channel;
      ChannelIdentifierBox m_identifier;
      ConnectionBox m_connection;
      ReaderBox m_reader;
      WriterBox m_writer;
  };

  template<typename T, typename... Args>
  ChannelBox::ChannelBox(std::in_place_type_t<T>, Args&&... args)
    : m_channel(std::make_shared<T>(std::forward<Args>(args)...)),
      m_identifier(&FullyDereference(std::static_pointer_cast<T>(
        m_channel)).GetIdentifier()),
      m_connection(&FullyDereference(std::static_pointer_cast<T>(
        m_channel)).GetConnection()),
      m_reader(&FullyDereference(std::static_pointer_cast<T>(
        m_channel)).GetReader()),
      m_writer(&FullyDereference(std::static_pointer_cast<T>(
        m_channel)).GetWriter()) {}

  template<typename Channel>
  ChannelBox::ChannelBox(Channel channel)
    : ChannelBox(std::in_place_type<Channel>, std::move(channel)) {}

  inline ChannelBox::ChannelBox(ChannelBox* channel)
    : ChannelBox(*channel) {}

  inline ChannelBox::ChannelBox(const std::shared_ptr<ChannelBox>& channel)
    : ChannelBox(*channel) {}

  inline ChannelBox::ChannelBox(const std::unique_ptr<ChannelBox>& channel)
    : ChannelBox(*channel) {}

  inline const ChannelBox::Identifier& ChannelBox::GetIdentifier() const {
    return m_identifier;
  }

  inline ChannelBox::Connection& ChannelBox::GetConnection() {
    return m_connection;
  }

  inline ChannelBox::Reader& ChannelBox::GetReader() {
    return m_reader;
  }

  inline ChannelBox::Writer& ChannelBox::GetWriter() {
    return m_writer;
  }
}

  template<>
  struct ImplementsConcept<IO::ChannelBox,
    IO::Channel<IO::ChannelIdentifierBox, IO::ConnectionBox, IO::ReaderBox,
      IO::WriterBox>> : std::true_type {};
}

#endif
