#ifndef BEAM_STOMPSERVER_HPP
#define BEAM_STOMPSERVER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Stomp/Stomp.hpp"
#include "Beam/Stomp/StompFrame.hpp"

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
      GetOptionalLocalPtr<ChannelType> m_channel;
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
    return StompFrame{StompCommand::EOL};
  }

  template<typename ChannelType>
  void StompServer<ChannelType>::Write(const StompFrame& frame) {}

  template<typename ChannelType>
  void StompServer<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_channel->GetConnection().Open();
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
