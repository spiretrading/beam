#ifndef BEAM_CHANNEL_ADAPTER_SERVER_CONNECTION_HPP
#define BEAM_CHANNEL_ADAPTER_SERVER_CONNECTION_HPP
#include <functional>
#include <type_traits>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /**
   * Adapter class for a ServerConnection requiring a specific Channel.
   * @param <S> A pointer to the type of ServerConnection to accept connections
   *        from.
   * @param <C> The type of Channel to adapt to.
   */
  template<typename S, typename C>
  class ChannelAdapterServerConnection {
    public:

      /** The type of ServerConnection to accept from. */
      using ServerConnection = std::decay_t<S>;

      /** The type of Channel to adapt from. */
      using SourceChannel = typename ServerConnection::Channel;

      /** The type of Channel to adapt to. */
      using Channel = C;

      /**
       * Defines the type of function used to adapt a Channel.
       * @param source The Channel to adapt.
       * @return The adapted Channel.
       */
      using Adapter = std::function<std::unique_ptr<Channel>(
        std::unique_ptr<SourceChannel> source)>;

      /**
       * Constructs a ChannelAdapterServerConnection.
       * @param connection The ServerConnection to adapt the Channel's from.
       * @param adapter The Adapter used to convert incoming Channels.
       */
      template<typename SF>
      ChannelAdapterServerConnection(SF&& connection, const Adapter& adapter);

      ~ChannelAdapterServerConnection();

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      GetOptionalLocalPtr<S> m_connection;
      Adapter m_adapter;

      ChannelAdapterServerConnection(
        const ChannelAdapterServerConnection&) = delete;
      ChannelAdapterServerConnection& operator =(
        const ChannelAdapterServerConnection&) = delete;
  };

  template<typename S, typename C>
  template<typename SF>
  ChannelAdapterServerConnection<S, C>::ChannelAdapterServerConnection(
    SF&& connection, const Adapter& adapter)
    : m_connection(std::forward<SF>(connection)),
      m_adapter(adapter) {}

  template<typename S, typename C>
  ChannelAdapterServerConnection<S, C>::~ChannelAdapterServerConnection() {
    Close();
  }

  template<typename S, typename C>
  std::unique_ptr<typename ChannelAdapterServerConnection<S, C>::Channel>
      ChannelAdapterServerConnection<S, C>::Accept() {
    return m_adapter(m_connection->Accept());
  }

  template<typename S, typename C>
  void ChannelAdapterServerConnection<S, C>::Close() {
    m_connection->Close();
  }
}

  template<typename S, typename C>
  struct ImplementsConcept<IO::ChannelAdapterServerConnection<S, C>,
    IO::ServerConnection<C>> : std::true_type {};
}

#endif
