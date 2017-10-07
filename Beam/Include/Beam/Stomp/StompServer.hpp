#ifndef BEAM_STOMPSERVER_HPP
#define BEAM_STOMPSERVER_HPP
#include <boost/noncopyable.hpp>
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

namespace Beam {
namespace Stomp {

  /*! \class StompServer
      \brief Represents the server side of the STOMP protocol.
      \tparam ChannelType The type of Channel to provide the protocol to.
   */
  template<typename ChannelType>
  class StompServer : private boost::noncopyable {
    public:

      //! The type of Channel to provide the protocol to.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! Constructs a StompServer.
      /*!
        \param channel The Channel to provide the protocol to.
      */
      template<typename ChannelForward>
      StompServer(ChannelForward&& channel);

      ~StompServer();

      //! Reads a frame from the Channel.
      StompFrame Read();

      //! Writes a frame to the Channel.
      /*!
        \param frame The frame to write.
      */
      void Write(const StompFrame& frame);

      void Open();

      void Close();

    private:
      mutable Threading::Mutex m_writeMutex;
      GetOptionalLocalPtr<ChannelType> m_channel;
      StompFrameParser m_parser;
      typename Channel::Reader::Buffer m_readBuffer;
      typename Channel::Writer::Buffer m_writeBuffer;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  StompServer<ChannelType>::StompServer(ChannelForward&& channel)
      : m_channel{std::forward<ChannelForward>(channel)} {}

  template<typename ChannelType>
  StompServer<ChannelType>::~StompServer() {
    Close();
  }

  template<typename ChannelType>
  StompFrame StompServer<ChannelType>::Read() {
    while(true) {
      auto frame = m_parser.GetNextFrame();
      if(frame.is_initialized()) {
        auto receipt = frame->FindHeader("receipt");
        if(receipt.is_initialized()) {
          StompFrame receiptFrame{StompCommand::RECEIPT};
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
            BOOST_THROW_EXCEPTION(IO::EndOfFileException{});
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

  template<typename ChannelType>
  void StompServer<ChannelType>::Write(const StompFrame& frame) {
    {
      boost::lock_guard<Threading::Mutex> lock{m_writeMutex};
      m_writeBuffer.Reset();
      Serialize(frame, Store(m_writeBuffer));
      m_channel->GetWriter().Write(m_writeBuffer);
    }
    if(frame.GetCommand() == StompCommand::ERR) {
      m_channel->GetConnection().Close();
    }
  }

  template<typename ChannelType>
  void StompServer<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_channel->GetConnection().Open();
      auto connectFrame = Read();
      if(connectFrame.GetCommand() != StompCommand::CONNECT &&
          connectFrame.GetCommand() != StompCommand::STOMP) {
        BOOST_THROW_EXCEPTION(IO::ConnectException{"CONNECT expected."});
      }
      StompFrame connectedFrame{StompCommand::CONNECTED};
      connectedFrame.AddHeader({"version", "1.2"});
      connectedFrame.AddHeader({"heart-beat", "0,0"});
      Write(connectedFrame);
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  void StompServer<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void StompServer<ChannelType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }
}
}

#endif
