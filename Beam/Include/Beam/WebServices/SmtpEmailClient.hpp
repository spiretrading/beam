#ifndef BEAM_SMTP_EMAIL_CLIENT_HPP
#define BEAM_SMTP_EMAIL_CLIENT_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailClient.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /**
   * Implements an email client using the SMTP protocol.
   * @param <C> The type of Channel connecting to the SMTP server.
   */
  template<typename C>
  class SmtpEmailClient : private boost::noncopyable {
    public:

      /** The type of Channel connecting to the SMTP server. */
      using Channel = GetTryDereferenceType<C>;

      /** The type used to build Channels to the SMTP server. */
      using ChannelBuilder = std::function<C ()>;

      /**
       * Constructs an SmtpEmailClient.
       * @param channelBuilder The ChannelBuilder to use.
       */
      SmtpEmailClient(ChannelBuilder channelBuilder);

      ~SmtpEmailClient();

      /**
       * Sets the username and password.
       * @param username The username.
       * @param password The password.
       */
      void SetCredentials(const std::string& username,
        const std::string& password);

      void Send(const Email& email);

      void Close();

    private:
      ChannelBuilder m_channelBuilder;
      boost::optional<std::string> m_username;
      boost::optional<std::string> m_password;
      IO::OpenState m_openState;
  };

  template<typename C>
  SmtpEmailClient<C>::SmtpEmailClient(ChannelBuilder channelBuilder)
    : m_channelBuilder(std::move(channelBuilder)) {}

  template<typename C>
  SmtpEmailClient<C>::~SmtpEmailClient() {
    Close();
  }

  template<typename C>
  void SmtpEmailClient<C>::SetCredentials(const std::string& username,
      const std::string& password) {
    m_username = Base64Encode(IO::BufferFromString<IO::SharedBuffer>(username));
    m_password = Base64Encode(IO::BufferFromString<IO::SharedBuffer>(password));
  }

  template<typename C>
  void SmtpEmailClient<C>::Send(const Email& email) {
    if(email.GetTo().empty()) {
      return;
    }
    auto channel = m_channelBuilder();
    using Channel = typename std::decay<decltype(*channel)>::type;
    auto ss = std::stringstream();
    auto reply = typename Channel::Reader::Buffer();
    auto WriteCommand = [&] {
      reply.Reset();
      auto command = ss.str();
      ss.str({});
      channel->GetWriter().Write(command.c_str(), command.size());
      channel->GetReader().Read(Store(reply));
    };
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

  template<typename C>
  void SmtpEmailClient<C>::Close() {
    m_openState.Close();
  }
}

#endif
