#ifndef BEAM_SERVER_CONNECTION_HPP
#define BEAM_SERVER_CONNECTION_HPP
#include <memory>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(ServerConnectionHasChannelType, Channel);
}

  /**
   * Interface for the server side of a Connection.
   * @param <C> The type of Channel to accept.
   */
  template<typename C>
  struct ServerConnection : Concept<ServerConnection<C>>, Concept<Connection> {
    static_assert(ImplementsConcept<C, IO::Channel<
      typename C::Identifier, typename C::Connection,
      typename C::Reader, typename C::Writer>>::value,
      "C must implement the Channel Concept.");

    /** Defines the type of Channel accepted by this server. */
    using Channel = C;

    /**
     * Accepts a new Channel.
     * @return The Channel that was accepted.
     */
    std::unique_ptr<Channel> Accept();
  };

  /**
   * Tests whether a type satisfies some particular ServerConnection Concept.
   * @param <T> The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsServerConnection : std::false_type {};

  template<typename T>
  struct IsServerConnection<T, std::enable_if_t<
    Details::ServerConnectionHasChannelType<T>::value>> :
    ImplementsConcept<T, ServerConnection<typename T::Channel>>::type {};
}
}

#endif
