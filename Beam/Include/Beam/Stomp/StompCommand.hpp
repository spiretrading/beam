#ifndef BEAM_STOMPCOMMAND_HPP
#define BEAM_STOMPCOMMAND_HPP
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

  inline const std::string& ToString(StompCommand command) {
    if(command == StompCommand::CONNECT) {
      static std::string value{"CONNECT"};
      return value;
    } else if(command == StompCommand::STOMP) {
      static std::string value{"STOMP"};
      return value;
    } else if(command == StompCommand::CONNECTED) {
      static std::string value{"CONNECTED"};
      return value;
    } else if(command == StompCommand::ERR) {
      static std::string value{"ERROR"};
      return value;
    } else if(command == StompCommand::EOL) {
      static std::string value{"\n"};
      return value;
    } else if(command == StompCommand::SEND) {
      static std::string value{"SEND"};
      return value;
    } else if(command == StompCommand::SUBSCRIBE) {
      static std::string value{"SUBSCRIBE"};
      return value;
    } else if(command == StompCommand::UNSUBSCRIBE) {
      static std::string value{"UNSUBSCRIBE"};
      return value;
    } else if(command == StompCommand::ACK) {
      static std::string value{"ACK"};
      return value;
    } else if(command == StompCommand::NACK) {
      static std::string value{"NACK"};
      return value;
    } else if(command == StompCommand::BEGIN) {
      static std::string value{"BEGIN"};
      return value;
    } else if(command == StompCommand::COMMIT) {
      static std::string value{"COMMIT"};
      return value;
    } else if(command == StompCommand::ABORT) {
      static std::string value{"ABORT"};
      return value;
    } else if(command == StompCommand::DISCONNECT) {
      static std::string value{"DISCONNECT"};
      return value;
    } else if(command == StompCommand::MESSAGE) {
      static std::string value{"MESSAGE"};
      return value;
    } else if(command == StompCommand::RECEIPT) {
      static std::string value{"RECEIPT"};
      return value;
    } else {
      BOOST_THROW_EXCEPTION(StompException{"Unknown command."});
    }
  }
}
}

#endif
