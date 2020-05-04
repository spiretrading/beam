#ifndef BEAM_SERVERCONNECTION_HPP
#define BEAM_SERVERCONNECTION_HPP
#include <memory>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(ServerConnectionHasChannelType, Channel);
}

  /*! \struct ServerConnection
      \brief Interface for the server side of a Connection.
      \tparam ChannelType The type of Channel to accept.
   */
  template<typename ChannelType>
  struct ServerConnection : Concept<ServerConnection<ChannelType>>,
      Concept<Connection> {
    static_assert(ImplementsConcept<ChannelType, IO::Channel<
      typename ChannelType::Identifier, typename ChannelType::Connection,
      typename ChannelType::Reader, typename ChannelType::Writer>>::value,
      "ChannelType must implement the Channel Concept.");

    //! Defines the type of Channel accepted by this server.
    using Channel = ChannelType;

    //! Accepts a new Channel.
    /*!
      \return The Channel that was accepted.
    */
    std::unique_ptr<Channel> Accept();
  };

  /*! \struct IsServerConnection
      \brief Tests whether a type satisfies some particular ServerConnection
             Concept.
      \tparam T The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsServerConnection : std::false_type {};

  template<typename T>
  struct IsServerConnection<T, typename std::enable_if<
    Details::ServerConnectionHasChannelType<T>::value>::type> :
    boost::mpl::if_c<ImplementsConcept<T, ServerConnection<
    typename T::Channel>>::value, std::true_type, std::false_type>::type {};
}
}

#endif
