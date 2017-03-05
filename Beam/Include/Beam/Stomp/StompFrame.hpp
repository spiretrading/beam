#ifndef BEAM_STOMPFRAME_HPP
#define BEAM_STOMPFRAME_HPP
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Stomp/StompCommand.hpp"
#include "Beam/Stomp/Stomp.hpp"
#include "Beam/Stomp/StompException.hpp"

namespace Beam {
namespace Stomp {

  /*! \class StompFrame
      \brief Represents a STOMP frame.
   */
  class StompFrame {
    public:
      StompFrame(StompCommand command);

    private:
      IO::SharedBuffer m_buffer;
  };

  inline StompFrame::StompFrame(StompCommand command) {
    if(command == StompCommand::CONNECT) {
      m_buffer.Append("CONNECT\n", 8);
    } else if(command == StompCommand::STOMP) {
      m_buffer.Append("STOMP\n", 6);
    } else if(command == StompCommand::STOMP) {
      m_buffer.Append("CONNECTED\n", 10);
    } else if(command == StompCommand::STOMP) {
      m_buffer.Append("ERROR\n", 6);
    } else if(command == StompCommand::EOL) {
      m_buffer.Append("\n", 1);
    } else if(command == StompCommand::SEND) {
      m_buffer.Append("SEND\n", 5);
    } else if(command == StompCommand::SUBSCRIBE) {
      m_buffer.Append("SUBSCRIBE\n", 10);
    } else if(command == StompCommand::UNSUBSCRIBE) {
      m_buffer.Append("UNSUBSCRIBE\n", 12);
    } else if(command == StompCommand::ACK) {
      m_buffer.Append("ACK\n", 4);
    } else if(command == StompCommand::NACK) {
      m_buffer.Append("NACK\n", 5);
    } else if(command == StompCommand::BEGIN) {
      m_buffer.Append("BEGIN\n", 6);
    } else if(command == StompCommand::COMMIT) {
      m_buffer.Append("COMMIT\n", 7);
    } else if(command == StompCommand::ABORT) {
      m_buffer.Append("ABORT\n", 6);
    } else if(command == StompCommand::DISCONNECT) {
      m_buffer.Append("DISCONNECT\n", 11);
    } else if(command == StompCommand::MESSAGE) {
      m_buffer.Append("MESSAGE\n", 8);
    } else if(command == StompCommand::RECEIPT) {
      m_buffer.Append("RECEIPT\n", 8);
    } else {
      BOOST_THROW_EXCEPTION(StompProtocolException{"Unknown command."});
    }
  }
}
}

#endif
