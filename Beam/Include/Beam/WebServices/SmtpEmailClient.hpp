#ifndef BEAM_SMTPEMAILCLIENT_HPP
#define BEAM_SMTPEMAILCLIENT_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailClient.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class SmtpEmailClient
      \brief Implements an email client using the SMTP protocol.
      \tparam ChannelType The type of Channel connecting to the SMTP server.
   */
  template<typename ChannelType>
  class SmtpEmailClient : private boost::noncopyable {
    public:

      //! The type of Channel connecting to the SMTP server.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! The type used to build Channels to the SMTP server.
      using ChannelBuilder = std::function<ChannelType ()>;

      //! Constructs an SmtpEmailClient.
      /*!
        \param channelBuilder The ChannelBuilder to use.
      */
      SmtpEmailClient(const ChannelBuilder& channelBuilder);

      ~SmtpEmailClient();

      //! Sets the username and password.
      /*!
        \param username The username.
        \param password The password.
      */
      void SetCredentials(const std::string& username,
        const std::string& password);

      void Send(const Email& email);

      void Open();

      void Close();

    private:
      ChannelBuilder m_channelBuilder;
      boost::optional<std::string> m_username;
      boost::optional<std::string> m_password;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  SmtpEmailClient<ChannelType>::SmtpEmailClient(
      const ChannelBuilder& channelBuilder)
      : m_channelBuilder{channelBuilder} {}

  template<typename ChannelType>
  SmtpEmailClient<ChannelType>::~SmtpEmailClient() {
    Close();
  }

  template<typename ChannelType>
  void SmtpEmailClient<ChannelType>::SetCredentials(const std::string& username,
      const std::string& password) {
    m_username = Base64Encode(IO::BufferFromString<IO::SharedBuffer>(username));
    m_password = Base64Encode(IO::BufferFromString<IO::SharedBuffer>(password));
  }

  template<typename ChannelType>
  void SmtpEmailClient<ChannelType>::Send(const Email& email) {
    if(email.GetTo().empty()) {
      return;
    }
    auto channel = m_channelBuilder();
    using Channel = typename std::decay<decltype(*channel)>::type;
    std::stringstream ss;
    typename Channel::Reader::Buffer reply;
    auto WriteCommand =
      [&] {
        reply.Reset();
        auto command = ss.str();
        ss.str({});
        channel->GetWriter().Write(command.c_str(), command.size());
        channel->GetReader().Read(Store(reply));
      };
    channel->GetConnection().Open();
    ss << "EHLO\r\n";
    WriteCommand();
    if(m_username.is_initialized()) {
      ss << "AUTH LOGIN\r\n";
      WriteCommand();
      ss << *m_username << "\r\n";
      WriteCommand();
      ss << *m_password << "\r\n";
      WriteCommand();
    }
    ss << "MAIL FROM:<" << email.GetFrom().GetAddress() << ">\r\n";
    WriteCommand();
    for(auto& recipient : email.GetTo()) {
      ss << "RCPT TO:<" << recipient.GetAddress() << ">\r\n";
      WriteCommand();
    }
    ss << "DATA\r\n";
    WriteCommand();
    ss << email;
    WriteCommand();
    ss << "QUIT\r\n";
    WriteCommand();
    channel->GetConnection().Close();
  }

  template<typename ChannelType>
  void SmtpEmailClient<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
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
    m_openState.SetClosed();
  }
}
}

#endif
