#ifndef BEAM_STOMP_SERVER_HPP
#define BEAM_STOMP_SERVER_HPP
#include <boost/throw_exception.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Stomp/Stomp.hpp"
#include "Beam/Stomp/StompFrame.hpp"
#include "Beam/Stomp/StompFrameParser.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::Stomp {

  /**
   * Represents the server side of the STOMP protocol.
   * @param <C> The type of Channel to provide the protocol to.
   */
  template<typename C>
  class StompServer {
    public:

      /** The type of Channel to provide the protocol to. */
      using Channel = GetTryDereferenceType<C>;

      /**
       * Constructs a StompServer.
       * @param channel The Channel to provide the protocol to.
       */
      template<typename CF>
      StompServer(CF&& channel);

      ~StompServer();

      /** Reads a frame from the Channel. */
      StompFrame Read();

      /**
       * Writes a frame to the Channel.
       * @param frame The frame to write.
       */
      void Write(const StompFrame& frame);

      void Close();

    private:
      mutable Threading::Mutex m_mutex;
      GetOptionalLocalPtr<C> m_channel;
      StompFrameParser m_parser;
      typename Channel::Reader::Buffer m_readBuffer;
      typename Channel::Writer::Buffer m_writeBuffer;
      IO::OpenState m_openState;

      StompServer(const StompServer&) = delete;
      StompServer& operator =(const StompServer&) = delete;
  };

  template<typename C>
  template<typename CF>
  StompServer<C>::StompServer(CF&& channel)
      : m_channel(std::forward<CF>(channel)) {
    try {
      auto connectFrame = Read();
      if(connectFrame.GetCommand() != StompCommand::CONNECT &&
          connectFrame.GetCommand() != StompCommand::STOMP) {
        BOOST_THROW_EXCEPTION(IO::ConnectException("CONNECT expected."));
      }
      auto connectedFrame = StompFrame(StompCommand::CONNECTED);
      connectedFrame.AddHeader({"version", "1.2"});
      connectedFrame.AddHeader({"heart-beat", "0,0"});
      Write(connectedFrame);
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  template<typename C>
  StompServer<C>::~StompServer() {
    Close();
  }

  template<typename C>
  StompFrame StompServer<C>::Read() {
    while(true) {
      if(auto frame = m_parser.GetNextFrame()) {
        if(auto receipt = frame->FindHeader("receipt")) {
          auto receiptFrame = StompFrame(StompCommand::RECEIPT);
          receiptFrame.AddHeader({"receipt-id", *receipt});
          try {
            Write(receiptFrame);
          } catch(const std::exception&) {}
        }
        if(frame->GetCommand() == StompCommand::EOL ||
            frame->GetCommand() == StompCommand::ACK ||
            frame->GetCommand() == StompCommand::NACK ||
            frame->GetCommand() == StompCommand::DISCONNECT) {
          if(frame->GetCommand() == StompCommand::DISCONNECT) {
            Close();
            BOOST_THROW_EXCEPTION(IO::EndOfFileException());
          }
          continue;
        }
        return *frame;
      }
      m_channel->GetReader().Read(Beam::Store(m_readBuffer));
      m_parser.Feed(m_readBuffer.GetData(), m_readBuffer.GetSize());
      m_readBuffer.Reset();
    }
  }

  template<typename C>
  void StompServer<C>::Write(const StompFrame& frame) {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_writeBuffer.Reset();
      Serialize(frame, Store(m_writeBuffer));
      m_channel->GetWriter().Write(m_writeBuffer);
    }
    if(frame.GetCommand() == StompCommand::ERR) {
      m_channel->GetConnection().Close();
    }
  }

  template<typename C>
  void StompServer<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }
}

#endif
