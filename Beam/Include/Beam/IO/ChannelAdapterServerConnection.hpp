#ifndef BEAM_CHANNELADAPTERSERVERCONNECTION_HPP
#define BEAM_CHANNELADAPTERSERVERCONNECTION_HPP
#include <functional>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class ChannelAdapterServerConnection
      \brief Adapter class for a ServerConnection requiring a specific Channel.
      \tparam ServerConnectionType A pointer to the type of ServerConnection to
                                   accept connections from.
      \tparam ChannelType The type of Channel to adapt to.
   */
  template<typename ServerConnectionType, typename ChannelType>
  class ChannelAdapterServerConnection : private boost::noncopyable {
    public:

      //! The type of ServerConnection to accept from.
      using ServerConnection = ServerConnectionType;

      //! The type of Channel to adapt from.
      using SourceChannel = typename ServerConnection::Channel;

      //! The type of Channel to adapt to.
      using Channel = ChannelType;

      //! Defines the type of function used to adapt a Channel.
      /*!
        \param source The Channel to adapt.
        \return The adapted Channel.
      */
      using Adapter = std::function<std::unique_ptr<Channel>(
        std::unique_ptr<SourceChannel>& source)>;

      //! Constructs a ChannelAdapterServerConnection.
      /*!
        \param connection The ServerConnection to adapt the Channel's from.
        \param adapter The Adapter used to convert incoming Channels.
      */
      template<typename ServerConnectionForward>
      ChannelAdapterServerConnection(ServerConnectionForward&& connection,
        const Adapter& adapter);

      ~ChannelAdapterServerConnection();

      std::unique_ptr<Channel> Accept();

      void Close();

    private:
      typename OptionalLocalPtr<ServerConnection>::type m_connection;
      Adapter m_adapter;
  };

  template<typename ServerConnectionType, typename ChannelType>
  template<typename ServerConnectionForward>
  ChannelAdapterServerConnection<ServerConnectionType, ChannelType>::
      ChannelAdapterServerConnection(ServerConnectionForward&& connection,
      const Adapter& adapter)
      : m_connection(std::forward<ServerConnectionForward>(connection)),
        m_adapter(adapter) {}

  template<typename ServerConnectionType, typename ChannelType>
  ChannelAdapterServerConnection<ServerConnectionType, ChannelType>::
      ~ChannelAdapterServerConnection() {
    Close();
  }

  template<typename ServerConnectionType, typename ChannelType>
  std::unique_ptr<typename ChannelAdapterServerConnection<ServerConnectionType,
      ChannelType>::Channel> ChannelAdapterServerConnection<
      ServerConnectionType, ChannelType>::Accept() {
    std::unique_ptr<SourceChannel> source = m_connection->Accept();
    std::unique_ptr<Channel> channel(m_adapter(source));
    return std::move(channel);
  }

  template<typename ServerConnectionType, typename ChannelType>
  void ChannelAdapterServerConnection<ServerConnectionType, ChannelType>::
      Close() {
    m_connection->Close();
  }
}

  template<typename ServerConnectionType, typename ChannelType>
  struct ImplementsConcept<IO::ChannelAdapterServerConnection<
    ServerConnectionType, ChannelType>, IO::ServerConnection<ChannelType>> :
    std::true_type {};
}

#endif
