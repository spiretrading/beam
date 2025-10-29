#ifndef BEAM_SMTP_EMAIL_CLIENT_HPP
#define BEAM_SMTP_EMAIL_CLIENT_HPP
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailClient.hpp"

namespace Beam {

  /**
   * Implements an email client using the SMTP protocol.
   * @tparam C The type of Channel connecting to the SMTP server.
   */
  template<typename C> requires IsChannel<dereference_t<C>>
  class SmtpEmailClient {
    public:

      /** The type of Channel connecting to the SMTP server. */
      using Channel = dereference_t<C>;

      /** The type used to build Channels to the SMTP server. */
      using ChannelBuilder = std::function<C ()>;

      /**
       * Constructs an SmtpEmailClient.
       * @param channel_builder The ChannelBuilder to use.
       */
      explicit SmtpEmailClient(ChannelBuilder channel_builder);

      ~SmtpEmailClient();

      /**
       * Sets the username and password.
       * @param username The username.
       * @param password The password.
       */
      void set_credentials(
        const std::string& username, const std::string& password);

      void send(const Email& email);
      void close();

    private:
      ChannelBuilder m_channel_builder;
      boost::optional<std::string> m_username;
      boost::optional<std::string> m_password;
      OpenState m_open_state;

      SmtpEmailClient(const SmtpEmailClient&) = delete;
      SmtpEmailClient& operator =(const SmtpEmailClient&) = delete;
  };

  template<typename F>
  SmtpEmailClient(F) -> SmtpEmailClient<std::invoke_result_t<F>>;

  template<typename C> requires IsChannel<dereference_t<C>>
  SmtpEmailClient<C>::SmtpEmailClient(ChannelBuilder channel_builder)
    : m_channel_builder(std::move(channel_builder)) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  SmtpEmailClient<C>::~SmtpEmailClient() {
    close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void SmtpEmailClient<C>::set_credentials(
      const std::string& username, const std::string& password) {
    m_username = encode_base64(from<SharedBuffer>(username));
    m_password = encode_base64(from<SharedBuffer>(password));
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void SmtpEmailClient<C>::send(const Email& email) {
    if(email.get_to().empty()) {
      return;
    }
    auto channel = m_channel_builder();
    auto ss = std::stringstream();
    auto reply = SharedBuffer();
    channel->get_reader().read(out(reply));
    auto write_command = [&] {
      reset(reply);
      auto command = ss.str();
      ss.str(std::string());
      channel->get_writer().write(from<SharedBuffer>(command));
      channel->get_reader().read(out(reply));
    };
    ss << "EHLO\r\n";
    write_command();
    if(m_username) {
      ss << "AUTH LOGIN\r\n";
      write_command();
      ss << *m_username << "\r\n";
      write_command();
      ss << *m_password << "\r\n";
      write_command();
    }
    ss << "MAIL FROM:<" << email.get_from().get_address() << ">\r\n";
    write_command();
    for(auto& recipient : email.get_to()) {
      ss << "RCPT TO:<" << recipient.get_address() << ">\r\n";
      write_command();
    }
    ss << "DATA\r\n";
    write_command();
    ss << email;
    write_command();
    ss << "QUIT\r\n";
    write_command();
    channel->get_connection().close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void SmtpEmailClient<C>::close() {
    m_open_state.close();
  }
}

#endif
