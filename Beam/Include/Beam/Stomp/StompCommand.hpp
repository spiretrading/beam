#ifndef BEAM_STOMPCOMMAND_HPP
#define BEAM_STOMPCOMMAND_HPP
#include "Beam/Stomp/Stomp.hpp"

namespace Beam {
namespace Stomp {

  /*! \enum StompCommand
      \brief Lists every STOMP frame command.
   */
  enum class StompCommand {

    //! Connects to a STOMP server.
    CONNECT,

    //! Alias for CONNECT.
    STOMP,

    //! Response from a STOMP server to a STOMP client upon connection.
    CONNECTED,

    //! Server indicates a protocol level error.
    ERROR,

    //! Empty frame.
    EOL,

    //! Sends data from the client to the server.
    SEND,

    //! Requests a subscription.
    SUBSCRIBE,

    //! Terminates a subscription.
    UNSUBSCRIBE,

    //! Acknowledges receipt of a message by a client.
    ACK,

    //! Indicates a message was not received by a client.
    NACK,

    //! Begins a transaction by the client.
    BEGIN,

    //! Commits a transaction by the client.
    COMMIT,

    //! Aborts a transaction by the client.
    ABORT,

    //! Client disconnects from the server.
    DISCONNECT,

    //! Server sends a message to the client.
    MESSAGE,

    //! Server signals receipt of a message from the client.
    RECEIPT,
  };
}
}

#endif
