#include <future>
#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailAddress.hpp"
#include "Beam/WebServices/SmtpEmailClient.hpp"

using namespace Beam;

TEST_SUITE("SmtpEmailClient") {
  TEST_CASE("send_basic_email") {
    auto server = LocalServerConnection();
    auto client = SmtpEmailClient([&] {
      return std::make_unique<LocalClientChannel>("smtp", server);
    });
    auto email = Email(EmailAddress("sender@example.com"),
      EmailAddress("recipient@example.com"));
    email.set_subject("Test Subject");
    email.add_body("Test message body");
    auto client_task = std::async(std::launch::async, [&] {
      client.send(email);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_writer().write(
      from<SharedBuffer>("220 smtp.example.com ESMTP\r\n"));
    channel->get_reader().read(out(buffer));
    auto command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "EHLO\r\n");
    reset(buffer);
    channel->get_writer().write(
      from<SharedBuffer>("250-smtp.example.com\r\n250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "MAIL FROM:<sender@example.com>\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "RCPT TO:<recipient@example.com>\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "DATA\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("354 Start mail input\r\n"));
    channel->get_reader().read(out(buffer));
    auto email_data = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(email_data.find("From: sender@example.com") != std::string::npos);
    REQUIRE(email_data.find("To: recipient@example.com") != std::string::npos);
    REQUIRE(email_data.find("Subject: Test Subject") != std::string::npos);
    REQUIRE(email_data.find("Test message body") != std::string::npos);
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "QUIT\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("221 Bye\r\n"));
    client_task.get();
  }

  TEST_CASE("send_with_authentication") {
    auto server = LocalServerConnection();
    auto client = SmtpEmailClient([&] {
      return std::make_unique<LocalClientChannel>("smtp", server);
    });
    client.set_credentials("username", "password");
    auto email = Email(EmailAddress("sender@example.com"),
      EmailAddress("recipient@example.com"));
    email.set_subject("Authenticated Email");
    email.add_body("Secure message");
    auto client_task = std::async(std::launch::async, [&] {
      client.send(email);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_writer().write(
      from<SharedBuffer>("220 smtp.example.com ESMTP\r\n"));
    channel->get_reader().read(out(buffer));
    auto command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "EHLO\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "AUTH LOGIN\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("334 VXNlcm5hbWU6\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command.find("dXNlcm5hbWU=") != std::string::npos);
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("334 UGFzc3dvcmQ6\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command.find("cGFzc3dvcmQ=") != std::string::npos);
    reset(buffer);
    channel->get_writer().write(
      from<SharedBuffer>("235 Authentication successful\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("354 Start mail input\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("221 Bye\r\n"));
    client_task.get();
  }

  TEST_CASE("send_to_multiple_recipients") {
    auto server = LocalServerConnection();
    auto client = SmtpEmailClient([&] {
      return std::make_unique<LocalClientChannel>("smtp", server);
    });
    auto email = Email(EmailAddress("sender@example.com"));
    email.add_to(EmailAddress("recipient1@example.com"));
    email.add_to(EmailAddress("recipient2@example.com"));
    email.add_to(EmailAddress("recipient3@example.com"));
    email.set_subject("Multiple Recipients");
    email.add_body("Message to all");
    auto client_task = std::async(std::launch::async, [&] {
      client.send(email);
    });
    auto channel = server.accept();
    auto buffer = SharedBuffer();
    channel->get_writer().write(
      from<SharedBuffer>("220 smtp.example.com ESMTP\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    auto command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "RCPT TO:<recipient1@example.com>\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "RCPT TO:<recipient2@example.com>\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    command = std::string(buffer.get_data(), buffer.get_size());
    REQUIRE(command == "RCPT TO:<recipient3@example.com>\r\n");
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("354 Start mail input\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("250 OK\r\n"));
    channel->get_reader().read(out(buffer));
    reset(buffer);
    channel->get_writer().write(from<SharedBuffer>("221 Bye\r\n"));
    client_task.get();
  }

  TEST_CASE("send_empty_recipient_list") {
    auto server = LocalServerConnection();
    auto client = SmtpEmailClient([&] {
      return std::make_unique<LocalClientChannel>("smtp", server);
    });
    auto email = Email(EmailAddress("sender@example.com"));
    email.set_subject("No Recipients");
    email.add_body("This should not be sent");
    client.send(email);
  }
}
