#ifndef BEAM_SMTPEMAILCLIENT_HPP
#define BEAM_SMTPEMAILCLIENT_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailClient.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class SmtpEmailClient.
      \brief Implements an email client using the SMTP protocol.
      \tparam ChannelType The type of Channel to use.
   */
  template<typename ChannelType>
  class SmtpEmailClient : private boost::noncopyable {
    public:

      //! The type of Channel to use.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! Constructs an SmtpEmailClient.
      /*!
        \param channel Initializes the Channel.
      */
      template<typename ChannelForward>
      SmtpEmailClient(ChannelForward&& channel);

      ~SmtpEmailClient();

      void Send(const Email& email);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<ChannelType> m_channel;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  SmtpEmailClient<ChannelType>::SmtpEmailClient(ChannelForward&& channel)
      : m_channel{std::forward<ChannelType>(channel)} {}

  template<typename ChannelType>
  SmtpEmailClient<ChannelType>::~SmtpEmailClient() {
    Close();
  }

  template<typename ChannelType>
  void SmtpEmailClient<ChannelType>::Send(const Email& email) {
  }

  template<typename ChannelType>
  void SmtpEmailClient<ChannelType>::Open() {
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
  void SmtpEmailClient<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void SmtpEmailClient<ChannelType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }
}
}

#endif
