#ifndef BEAM_CHANNEL_HPP
#define BEAM_CHANNEL_HPP
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/IO/ChannelIdentifier.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept satisfied by types that implement the Channel interface. */
  template<typename T>
  concept IsChannel = requires(T& t) {
    typename T::Identifier;
    typename T::Connection;
    typename T::Reader;
    typename T::Writer;
    { t.get_identifier() } -> std::same_as<const typename T::Identifier&>;
    { t.get_connection() } -> std::same_as<typename T::Connection&>;
    { t.get_reader() } -> std::same_as<typename T::Reader&>;
    { t.get_writer() } -> std::same_as<typename T::Writer&>;
  } && IsChannelIdentifier<typename T::Identifier> &&
    IsConnection<typename T::Connection> && IsReader<typename T::Reader> &&
    IsWriter<typename T::Writer>;

  /** Composes a Connection, a Reader, and a Writer into an IO channel. */
  class Channel {
    public:

      /** The type of ChannelIdentifier. */
      using Identifier = Beam::ChannelIdentifier;

      /** The type of Connection. */
      using Connection = Beam::Connection;

      /** The type of Reader. */
      using Reader = Beam::Reader;

      /** The type of Writer. */
      using Writer = Beam::Writer;

      /**
       * Constructs a Channel of a specified type using emplacement.
       * @tparam T The type of channel to emplace.
       * @param args The arguments to pass to the emplaced channel.
       */
      template<IsChannel T, typename... Args>
      explicit Channel(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Channel by referencing an existing channel.
       * @param channel The channel to reference.
       */
      template<DisableCopy<Channel> T> requires IsChannel<dereference_t<T>>
      Channel(T&& channel);

      Channel(const Channel&) = default;
      Channel(Channel&&) = default;

      /** Returns the Channel's identifier. */
      const Identifier& get_identifier() const;

      /** Returns the Connection. */
      Connection& get_connection();

      /** Returns the Reader. */
      Reader& get_reader();

      /** Returns the Writer. */
      Writer& get_writer();

    private:
      struct VirtualChannel {
        virtual ~VirtualChannel() = default;

        virtual const Identifier& get_identifier() const = 0;
        virtual Connection& get_connection() = 0;
        virtual Reader& get_reader() = 0;
        virtual Writer& get_writer() = 0;
      };
      template<typename C>
      struct WrappedChannel final : VirtualChannel {
        using Channel = C;
        local_ptr_t<Channel> m_channel;
        Identifier m_identifier;
        Connection m_connection;
        Reader m_reader;
        Writer m_writer;

        template<typename... Args>
        WrappedChannel(Args&&... args);

        const Identifier& get_identifier() const override;
        Connection& get_connection() override;
        Reader& get_reader() override;
        Writer& get_writer() override;
      };
      VirtualPtr<VirtualChannel> m_channel;
  };

  template<IsChannel T, typename... Args>
  Channel::Channel(std::in_place_type_t<T>, Args&&... args)
    : m_channel(
        make_virtual_ptr<WrappedChannel<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Channel> T> requires IsChannel<dereference_t<T>>
  Channel::Channel(T&& channel)
    : m_channel(make_virtual_ptr<WrappedChannel<std::remove_cvref_t<T>>>(
        std::forward<T>(channel))) {}

  inline const Channel::Identifier& Channel::get_identifier() const {
    return m_channel->get_identifier();
  }

  inline Channel::Connection& Channel::get_connection() {
    return m_channel->get_connection();
  }

  inline Channel::Reader& Channel::get_reader() {
    return m_channel->get_reader();
  }

  inline Channel::Writer& Channel::get_writer() {
    return m_channel->get_writer();
  }

  template<typename C>
  template<typename... Args>
  Channel::WrappedChannel<C>::WrappedChannel(Args&&... args)
    : m_channel(std::forward<Args>(args)...),
      m_identifier(m_channel->get_identifier()),
      m_connection(&m_channel->get_connection()),
      m_reader(&m_channel->get_reader()),
      m_writer(&m_channel->get_writer()) {}

  template<typename C>
  const Channel::Identifier&
      Channel::WrappedChannel<C>::get_identifier() const {
    return m_identifier;
  }

  template<typename C>
  Channel::Connection& Channel::WrappedChannel<C>::get_connection() {
    return m_connection;
  }

  template<typename C>
  Channel::Reader& Channel::WrappedChannel<C>::get_reader() {
    return m_reader;
  }

  template<typename C>
  Channel::Writer& Channel::WrappedChannel<C>::get_writer() {
    return m_writer;
  }
}

#endif
