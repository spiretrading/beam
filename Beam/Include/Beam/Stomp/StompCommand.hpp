#ifndef BEAM_STOMPCOMMAND_HPP
#define BEAM_STOMPCOMMAND_HPP
#include <ostream>
#include <boost/throw_exception.hpp>
#include "Beam/Stomp/Stomp.hpp"
#include "Beam/Stomp/StompException.hpp"

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
    ERR,

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

  inline std::ostream& operator <<(std::ostream& out, StompCommand command) {
    if(command == StompCommand::CONNECT) {
      return out << "CONNECT";
    } else if(command == StompCommand::STOMP) {
      return out << "STOMP";
    } else if(command == StompCommand::CONNECTED) {
      return out << "CONNECTED";
    } else if(command == StompCommand::ERR) {
      return out << "ERROR";
    } else if(command == StompCommand::EOL) {
      return out << '\n';
    } else if(command == StompCommand::SEND) {
      return out << "SEND";
    } else if(command == StompCommand::SUBSCRIBE) {
      return out << "SUBSCRIBE";
    } else if(command == StompCommand::UNSUBSCRIBE) {
      return out << "UNSUBSCRIBE";
    } else if(command == StompCommand::ACK) {
      return out << "ACK";
    } else if(command == StompCommand::NACK) {
      return out << "NACK";
    } else if(command == StompCommand::BEGIN) {
      return out << "BEGIN";
    } else if(command == StompCommand::COMMIT) {
      return out << "COMMIT";
    } else if(command == StompCommand::ABORT) {
      return out << "ABORT";
    } else if(command == StompCommand::DISCONNECT) {
      return out << "DISCONNECT";
    } else if(command == StompCommand::MESSAGE) {
      return out << "MESSAGE";
    } else if(command == StompCommand::RECEIPT) {
      return out << "RECEIPT";
    } else {
      return out << "NONE";
    }
  }
}
}

#endif
