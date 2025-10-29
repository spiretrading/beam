#ifndef BEAM_CHANNEL_ADAPTER_SERVER_CONNECTION_HPP
#define BEAM_CHANNEL_ADAPTER_SERVER_CONNECTION_HPP
#include <functional>
#include <type_traits>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /**
   * Adapter class for a ServerConnection requiring a specific Channel.
   * @tparam S The type of ServerConnection to accept connections from.
   * @tparam C The type of Channel to adapt to.
   */
  template<IsServerConnection S, IsChannel C>
  class ChannelAdapterServerConnection {
    public:

      /** The type of ServerConnection to accept from. */
      using ServerConnection = S;

      /** The type of Channel to adapt from. */
      using SourceChannel = typename ServerConnection::Channel;

      /** The type of Channel to adapt to. */
      using Channel = C;

      /**
       * Defines the type of function used to adapt a Channel.
       * @param source The Channel to adapt.
       * @return The adapted Channel.
       */
      using Adapter = std::function<
        std::unique_ptr<Channel> (std::unique_ptr<SourceChannel> source)>;

      /**
       * Constructs a ChannelAdapterServerConnection.
       * @param connection The ServerConnection to adapt the Channel's from.
       * @param adapter The Adapter used to convert incoming Channels.
       */
      template<Initializes<S> SF>
      ChannelAdapterServerConnection(SF&& connection, const Adapter& adapter);

      ~ChannelAdapterServerConnection();

      std::unique_ptr<Channel> accept();
      void close();

    private:
      local_ptr_t<S> m_connection;
      Adapter m_adapter;

      ChannelAdapterServerConnection(
        const ChannelAdapterServerConnection&) = delete;
      ChannelAdapterServerConnection& operator =(
        const ChannelAdapterServerConnection&) = delete;
  };

  template<IsServerConnection S, IsChannel C>
  template<Initializes<S> SF>
  ChannelAdapterServerConnection<S, C>::ChannelAdapterServerConnection(
    SF&& connection, const Adapter& adapter)
    : m_connection(std::forward<SF>(connection)),
      m_adapter(adapter) {}

  template<IsServerConnection S, IsChannel C>
  ChannelAdapterServerConnection<S, C>::~ChannelAdapterServerConnection() {
    close();
  }

  template<IsServerConnection S, IsChannel C>
  std::unique_ptr<typename ChannelAdapterServerConnection<S, C>::Channel>
      ChannelAdapterServerConnection<S, C>::accept() {
    try {
      return m_adapter(m_connection->accept());
    } catch(const std::exception&) {
      std::throw_with_nested(IOException());
    }
  }

  template<IsServerConnection S, IsChannel C>
  void ChannelAdapterServerConnection<S, C>::close() {
    m_connection->close();
  }
}

#endif
